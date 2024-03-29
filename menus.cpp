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
// menus.cpp
// Project: Postal
//
// This module contains the actual menu data (structs, strings, etc.)
//
// History:
//		12/05/96 MJR	Started.
//
//		12/19/96	JMI	Menus contained a pointer to guiItem which was a not-yet-
//							created gui from my old menu sample code.  Removed ptr.
//							Typo in name of MainMenuTile.bmp.
//
//		12/19/96	JMI	Offset menu items so they didn't overwrite menu heading
//							and moved "Ready?" from verify exit menu to the heading.
//
//		01/27/97	JMI	Added more menus (High Scores, Options, and Start).
//
//		01/28/97	JMI	Added new menu background and indicator for two menus
//							(Main and VerifyExit).
//
//		01/28/97	JMI	Was setting menu box to not transparent instead of trans-
//							parent.  Fixed.
//
//		01/29/97	JMI	Now uses SampleMaster to play a sample when a choice is
//							made from the main menu.
//
//		01/30/97	JMI	Added cheezy brighten/darken effect via scrollbar in 
//							options menu.
//
//		01/31/97	JMI	Made all menus uppercase.  Also, uses flesh menu and
//							smash font for all menus now.  Adjusted menu paths
//							so they are in the res/Menus dir and font paths so
//							they are in the res/Fonts dir.
//
//		01/31/97	JMI	Adjusted menu sizes such that the flesh menu could be 
//							shown.
//
//		02/02/97	JMI	Added support for -1 passed on MenuChoice callback 
//							indicating change in focused menu item.  Added use of
//							new child struct in Menu struct, MenuGui.  Relocated
//							title text on some menus.  Resized font on some menus
//							to fit better.  Got rid of PalTran func which was doing
//							nothing now that game.cpp does CLUT red shift/fade.
//
//		02/03/97	JMI	Now menuClientGame invokes the options menu, if selected.
//							Adjusted paths for SAK dir.
//
//		02/04/97	JMI	Now uses functions in game.cpp to get and set the gamma
//							level.
//
//		02/04/97	JMI	Forgot to release memfile used to get scrollbar for options
//							menu.
//
//		02/10/97	JMI	rspReleaseResource() now takes a ptr to a ptr.
//
//		02/19/97	JMI	More implementation for Start Game menu.
//
//		03/06/97	JMI	Added g_menuVerifyQuitGame.
//
//		03/07/97	JMI	Added Load menu option on main menu.
//
//		03/28/97	JMI	Disabled help and ordering info.
//
//		04/07/97	JMI	When I added the Load menu option, I must've screwed up
//							the StartGameMenu() choice function so that it always
//							started a game.  Fixed.
//
//		04/07/97	JMI	Now uses g_GameSettings.m_szServerName for the server name.
//
//		04/08/97	JMI	Added multiplayer options menu (menuMultiOptions).
//
//		04/11/97	JMI	Added record and play demo options to Start Game Menu.
//
//		04/14/97	JMI	Swapped positions of Record and Play menu options.
//
//		04/21/97	JMI	Added menuEditor and utilized new auto go back 
//							functionality in Menu API.
//
//		04/21/97	JMI	menuEditor now specifies the palette indices to be set by
//							the Menu API.
//
//		04/22/97	JMI	Added controls menu and sub menus for keyboard, mouse, and
//							joystick.  Only the mouse sub menu is partially implemented
//							functionally (the rest are not yet implemented).
//
//		04/22/97	JMI	Added checkbox for enabling mouse input.
//
//		04/22/97	JMI	Separated game start options into 3 categories, single
//							player, multiplayer, and demo.
//
//		04/22/97	JMI	Updated menuClient to work as the in game menu for Play.cpp.
//
//		04/22/97	JMI	No longer uses chdir() to get the open dialog into the
//							correct directory.  Now we simply default szRealmName
//							to the path of the dir we want to save under.
//
//		04/22/97	JMI	Now VerifyQuitMenuChoice handles menuClientGame instead of
//							g_menuVerifyQuit.
//
//		04/24/97	JMI	Took out the ! in "Postal!" title of main menu.
//
//		04/24/97	JMI	Now uses the centering flag for header text and 
//							two new flags for shadowing the header and the items.
//							Now colors are specified as RGBA and background font
//							color must be transparent (not offered as an option).
//
//		04/24/97	JMI	Change menuClientGame's "GAME MENU" to "MENU".
//
//		04/25/97	JMI	Changed menu item text shadow color to be more blood like.
//							Unified all menu header and item text sizes.
//							Scooted menu items in a bit to avoid having the bullet in-
//							dicator go off the menu.
//
//		04/25/97	JMI	StartSingleMenu() was not calling Play_SetRealmName(NULL)
//							to clear the realm name so the game went in level order.
//
//		05/06/97	JMI	Now the Gamma GUI displays text next to the scrollbar.
//
//		05/07/97	JMI	Now the range of the Gamma GUI text is -50 to +50.
//
//		05/15/97	JMI	Added the menu for disabling features like alpha blending
//							and X-ray effect.
//
//		05/22/97	JMI	Added 3D lighting to the features that can be disabled.
//
//		05/23/97	JMI	Added particle effects to the features that can be disabled.
//
//		05/25/97	JMI	Swapped positions of Join and Host on Multiplayer menu.
//
//		05/29/97	JMI	Now, when g_GameSettings.m_sServer is set, 
//							g_GameSettings.m_sClient is set as well.
//							
//		06/11/97	JMI	Added g_MenuSettings.
//
//		06/12/97 MJR	Moved much of the callback code from here into game.cpp.
//
//		07/01/97	JMI	Now plays a different sound for a selected item vs. an
//							item selection change.
//
//		07/03/97	JMI	Now calls Game_ControlsMenu() with choices from 
//							menuControls.
//
//		07/04/97	JMI	Added the appropriate amount of items for the keyboard,
//							mouse, and joystick menus.
//							Also, moved 'Use Mouse' to Options menu.
//							Also, changed "CONTROLS" option on Options menu to "EDIT
//							CONTROLS", "KEYBOARD" to "EDIT KEYS", and collapsed
//							"MOUSE" & "JOYSTICK" into "EDIT BUTTONS".
//
//		07/07/97	JMI	Added callbacks for menuJoystick.
//
//		07/12/97	JMI	Removed menuHighScores and its reference by menuMain.  
//							Also, adjusted MenuChoice switch values to compensate for
//							choices after 'High Scores' being pushed back (3..5 
//							became 2..4).
//
//		07/13/97	JMI	Added Challenge option from menuStart that leads to new
//							menuChallenge menu.
//
//		07/14/97	JMI	Added difficulty slider to menuOptions.
//
//		07/16/97	JMI	Added menuVolumes and path from menuOptions.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//							Also, added an '_' in "START GAME".
//
//		07/18/97	JMI	Changed "LOAD" to "LOAD_LEVEL" and added some underscores
//							for spaces in "LOAD GAME" and "THE GAUNTLET".
//
//		07/19/97	JMI	Added 'Volume Distancing' to menuFeatures.
//							Now 'Options' launches many other menus for the options:
//							menuVideo, menuAudio, menuPlayOptions, menuInput.
//
//		07/21/97	JMI	Changed "FEATURES" menu to "PERFORMANCE".
//
//		07/29/97	JMI	Now uses in-thumb area for text representation of current
//							gamma value.  The gamma range can be adjusted from the
//							scrollbar file like before, but now, the range in the
//							scrollbar will be mapped to the range GAMMA_RANGE so the
//							user's control can be more or less granular than the actual
//							gamma range.
//
//		08/01/97	JMI	Now there's a 'SETUP' option on the menuStartMulti that
//							goes to menuMultiOptions.
//							Also, changed 'Other' in multi options menu to net port.
//
//		08/03/97 BRH	Added network protocol option to the multiplayer setup
//							menu.
//
//		08/04/97	JMI	Added sliders for mouse sensitivities (X & Y).
//							Added menu with sliders for all facets of rotation.
//
//		08/04/97	JMI	Added performance option for playing ambient sounds.
//
//		08/05/97	JMI	Added multiplayer option for connection type.
//
//		08/05/97	JMI	Added 'Touch Organ' option to menuAudioOptions.
//
//		08/06/97	JMI	Changed "MOUSE SENS. X" and "... Y" to better names and
//							use Columnize Guis flag on menuControls.
//
//		08/07/97 BRH	Fixed problem where the port numnber was not getting
//							set from the gui.
//
//		08/12/97	JMI	Added some '...'s to try that concept with menus where you
//							want to know.
//
//		08/13/97 MJR	Added Host Name gui.
//
//		08/14/97	JMI	Converted menu callbacks to returning true to accept or
//							false to deny.
//
//		08/15/97	JRD	Added separate postal organ menu,
//
//		08/17/97 BRH	Got rid of violence option on PlayOptions menu, and
//							got rid of help on the main menu.
//
//		08/17/97	JMI	Now releases protocol chooser GUI.
//
//		08/17/97	JMI	menuOrgan was calling AudioOptionsInit() and 
//							AudioOptionsChoice() so choices on the organ menu were
//							affecting audio options.  Changed to no init and
//							Organ_MenuChoice() as the choice callback.
//							Also, added two lines explaining briefly that you can hit
//							A..Z and 0..9 to play with the organ.
//
//		08/17/97	JMI	Changed wording on menuOrgan.
//
//		08/17/97	MJR	Now loads from g_resmgrShell, and uses new menu bg's.
//
//		08/18/97 BRH	Changed protocol gui from an REdit to an RTxt gui.
//
//		08/20/97 BRH	Added new samples for the new pain and suffering volume
//							adjustment categories.
//
//					MJR	Changed multiplayer-related menus and added a new one.
//
//		08/23/97	JMI	Now the init/kill callback for the main menu calls game.cpp
//							to hook it in.
//
//		08/23/97	JMI	Added better player color menu feedback.
//
//		08/23/97	JMI	Moved ms_apszPlayerColorDescriptions[] from here into
//							CGameSettings where it can be used more generally.
//
//		08/24/97 MJR	Lots of tuning for consistancy, wording, etc.
//							More and more tuning.
//							Removed grip-zone-radius stuff -- no longer needed.
//
//		08/25/97 MJR	Made the color stuff into macros so they could easily be
//							changed for all menus.  Changed the RGB values, but the
//							color mapping didn't seem to be affected!!!
//
//		08/25/97	JMI	Colors were mapping to windows static colors so they did 
//							not apparently change.  Added mappable range to all menus
//							and made it so all menus set the palette as well.
//							Also, added code for new difficulty GUI which provides a
//							slider and a text area.
//							Also, changed the unspecified sound category sampler to 
//							the mine beep.
//							Also, now scales the ranges of the sound sliders to between
//							0 to 10 for the user and sets the value scaled to the selected 
//							category's adjusted value for the current quality.
//
//		08/26/97	JMI	Changed difficulty strings and name of PLAY menu to 
//							DIFFICULTY.  Also, changed the DIFFICULTY item to something
//							shorter like SET.
//
//		08/27/97	JMI	Now when you choose 'Defaults' from the Mixer menu, it
//							plays all the samples at the default volume for feedback
//							(rather than just the last one).
//
//		08/27/97	JMI	Took all underscores out of menu titles and items now that
//							RPrefs supports spaces in vars and sections.
//
//		08/27/97	JMI	Changed "PLAY" option on Options menu to "DIFFICULTY".
//
//		08/27/97	JMI	Now displays rotation degree values as integrals. 
//							Also, all menus no use g_fontPostal for their helper GUIs.
//
//		08/27/97	JMI	Removed "BACK" menu items.  Actually replaced with a
//							disabled "".  Note that this is not required by the menu 
//							system.  That is, you can set the cancel item to an item
//							that doesn't exist.  The problem is there's no mechanism for
//							specifying the one after the last item and so each menu would
//							have to constantly be updated.
//							Also, now enter on any item in the mixer will play all the
//							sounds simultaneously so you can get an idea of their
//							combined levels.
//							Also, changed sampler for Unspecified to the empty weapon
//							noise (was mine beep).
//
//		09/02/97	JMI	Added browse option to menuStartDemo.
//
//		09/04/97	JMI	Removed browse option from menuStartDemo.
//
//		09/06/97	JMI	No longer displays the Demo menu even for an instant if
//							g_GameSettings.m_sCanRecordDemos is FALSE.
//
//		09/17/97	JMI	Menus now use localizable string variables from localize.*.
//
//		09/26/97	JMI	Now uses the 'Restore defaults' string from localize.
//
//		09/26/97	JMI	Resized menus to fit localization text for French & German:
//
//							Menu							Change		Language that caused
//							======================	==========	=======================
//							g_menuVerifyQuitGame		SM -> MD		French, German
//							menuVerifyExit				SM -> MD		French, German
//							menuOptions					SM -> MD		French, German
//							menuRotation				MD -> LG		French
//							menuStart					SM -> MD		French, German
//							menuStartSingle			SM -> MD		French, German
//							menuStartMulti				SM -> MD		French, German
//							menuChallenge 				SM -> MD		German
//							menuJoinMulti				MD -> LG		French, German
//
//		10/10/97	JMI	Put menuJoystick back on menuControls based on conditional
//							compile option ALLOW_JOYSTICK.  Also, updated 
//							ControlsChoice() amd ControlsInit() to handle these cases.
//
//		10/13/97	JMI	Moved g_pszControlsMenu_UseJoystick to localize.* and took
//							out 'TEMP' comments.
//							Also, made 'Controls' menu a large menu b/c the new 
//							joystick options were pushing things off the edge.
//
//		12/01/97 BRH	Added #ifdef SPAWN sections to menus so that that spawn
//							verison of the game has different menus that only allow
//							them to join a multiplayer game.
//
//		12/04/97 BRH	Added additional option for single player missions so they
//							can play the original postal levels or the add on pack
//							levels.
//
//		10/07/99	JMI	Conditional remove Add On start menu option when in 
//							SUPER_POSTAL target.
//
//		03/30/00 MJR	Switched to START_MENU_ADDON_ITEM to control whether
//							the START menu has an Add-On item.
//
////////////////////////////////////////////////////////////////////////////////
#define MENUS_CPP

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/Menu/menu.h"
#else
	#include "menu.h"
#endif
#include "menus.h"
#include "game.h"
#include "play.h"
#include "SampleMaster.h"
#include "input.h"
#include "gameedit.h"
#include "input.h"
#include "MenuSettings.h"
#include "InputSettingsDlg.h"
#include "socket.h"
#include "organ.h"
#include "net.h"
#include "CompileOptions.h"	// For ALLOW_JOYSTICK macro.
#include "update.h"

#ifdef WIN32
	#include <direct.h>
#else
	#include <unistd.h>
#endif


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define MENU_BG_SM						"Menu/menu_sm.bmp"
#define MENU_BG_MD						"Menu/menu_md.bmp"
#define MENU_BG_LG						"Menu/menu_lg.bmp"

#define MENU_RECT_SM						20, 20, 280, 350
#define MENU_HEAD_Y_SM					-149
#define MENU_ITEM_X_SM					70
#define MENU_ITEM_Y_SM					-101
#define MENU_ITEM_SPACE_Y_SM			4
#define MENU_ITEM_IND_SPACE_X_SM		5

#define MENU_RECT_MD						20, 20, 450, 460
#define MENU_HEAD_Y_MD					-196
#define MENU_ITEM_X_MD					60
#define MENU_ITEM_Y_MD					-140
#define MENU_ITEM_SPACE_Y_MD			4
#define MENU_ITEM_IND_SPACE_X_MD		5

#define MENU_RECT_LG						0,   0, 640, 480
#define MENU_HEAD_Y_LG					-195
#define MENU_ITEM_X_LG					60
#define MENU_ITEM_Y_LG					-95
#define MENU_ITEM_SPACE_Y_LG			4
#define MENU_ITEM_IND_SPACE_X_LG		5

#define MENU_INDICATOR					"Menu/bullet.bmp"

#define SMASH_FONT						"Fonts/smash.fnt"
#define HEAD_FONT_HEIGHT				36
#define ITEM_FONT_HEIGHT				28
#define ITEM_FONT_HEIGHT_SMALLER		24

// This is used when setting up font's for GUI's.  The size doesn't matter, but we do have to
// supply a value, so just for kicks, let's use the same one each time.  Since we use RFont to
// try to figure out how many font sizes are used, we want this to be a cached size.
#define DEFAULT_GUI_FONT_HEIGHT		19

#define HEAD_COLOR						MAKE_U32_COLOR(160,   0,   0,   0)
#define HEAD_SHADOW_COLOR				MAKE_U32_COLOR( 64,  16,  16,   0)
#define ITEM_COLOR						MAKE_U32_COLOR(160,   0,   0,   0)
#define ITEM_SHADOW_COLOR				MAKE_U32_COLOR( 64,  16,  16,   0)

// Background color index for where there is no menu graphic.
#define MENU_BG_COLOR					252

// Starting palette entry to set.
#define PAL_SET_START					0
// Number of palette entries to set.
#define PAL_SET_NUM						230

// Starting palette entry we can map menu colors to.
#define PAL_MAP_START					10
// Number of palette entries we can map menu colors to.
#define PAL_MAP_NUM						236

#define GUI_GAMMA_FILE					"Menu/GammaSB.gui"
#define GUI_CONNECT_IP_FILE			"Menu/JoinIP.gui"
#define GUI_HOST_NAME_FILE				"Menu/HostName.gui"
#define PLAYER_NAME_GUI_FILE			"Menu/PlayerName.gui"
#define PLAYER_COLOR_GUI_FILE			"Menu/PlayerColor.gui"
#define GUI_CHECKBOX_FILE				"Menu/CheckBox0.gui"
#define GUI_DIFFICULTY_FILE			"Menu/Difficulty.gui"
#define GUI_VOLUME_FILE					"Menu/VolumeSB.gui"
#define NET_PORT_GUI_FILE				"Menu/NetPort.gui"
#define NET_PROTO_GUI_FILE				"Menu/NetProto.gui"
#define NET_CONNECTION_GUI_FILE		"Menu/NetConnectionTxt.gui"
#define GUI_MOUSE_SENSITIVITY_FILE	"Menu/MouseSensitivitySB.gui"
#define GUI_ROTATION_FILE				"Menu/RotationSB.gui"
#define GUI_TAP_ROTATION_FILE			"Menu/TapRotationSB.gui"

#define GUI_ID_GRIPRADIUS_VAL			10
#define GUI_ID_GAMMA_VAL				10
#define GUI_ID_SENSITIVITY_VAL		10
#define GUI_ID_ROTATION_VAL			10
#define GUI_ID_VOLUME_VAL				10

#define GUI_ID_DIFFICULTY_SLIDER		8000
#define GUI_ID_DIFFICULTY_TEXT		8001
#define GUI_ID_DIFFICULTY_VAL			8002

#define MIN_GAMMA_VAL					50
#define MAX_GAMMA_VAL					205
#define GAMMA_RANGE						(MAX_GAMMA_VAL - MIN_GAMMA_VAL)

#define USER_VOLUME_RANGE				11

#define MOUSE_SENSITIVITY_DIVISOR	5.0

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)					(sizeof(a) / sizeof(a[0]) )


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Menu callbacks.

static int16_t MainMenuInit(		// Returns 0 on succes, non-zero to cancel menu.
	Menu*	pmenuCurrent,			// In:  Menu being init'ed or killed.
	int16_t sInit);					// In:  TRUE, if initializing; FALSE, if killing.

static bool MainMenuChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.
static bool VerifyExitMenuChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static bool ClientGameMenuChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static bool EditorMenuChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static bool StartGameMenu(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t StartGameInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool NewCampaignMenu(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t NewCampaignInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.


static bool StartSingleMenu(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t ChallengeInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool ChallengeChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static bool ChallengeTimedChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static bool ChallengeGoalChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static bool ChallengeFlagChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static bool ChallengeCheckpointChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t StartSingleInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool StartMultiMenu(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t StartMultiInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool JoinMultiMenu(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t JoinMultiInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool HostMultiMenu(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t HostMultiInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool StartDemoMenu(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t StartDemoInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static int16_t OptionsInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool OptionsChoice(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t GameOptionsInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool GameOptionsChoice(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.
	
static int16_t PlayOptionsInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool PlayOptionsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,				// Current menu.
	int16_t	sMenuItem);					// Item chosen.

static int16_t VideoOptionsInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,					// Current menu.
	int16_t	sInit);						// TRUE, if initializing; FALSE, if killing.

static bool VideoOptionsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,				// Current menu.
	int16_t	sMenuItem);					// Item chosen.

static int16_t AudioOptionsInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,					// Current menu.
	int16_t	sInit);						// TRUE, if initializing; FALSE, if killing.

static bool AudioOptionsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,				// Current menu.
	int16_t	sMenuItem);					// Item chosen.

static int16_t VolumesInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool VolumesChoice(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t ControlsInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool ControlsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t RotationInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool RotationChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t MouseInit(			// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool MouseChoice(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t KeyInit(			// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool KeyChoice(			// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t JoyInit(			// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool JoyChoice(			// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);				// Item chosen.

static int16_t LoadLevelInit(			// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool LoadLevelChoice(			// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem);

static int16_t MultiOptionsInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,					// Current menu.
	int16_t	sInit);						// TRUE, if initializing; FALSE, if killing.

static bool MultiOptionsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCur,					// Current menu.
	int16_t	sMenuItem);					// Item chosen.

static int16_t FeaturesInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit);					// TRUE, if initializing; FALSE, if killing.

static bool FeaturesChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sMenuItem);				// Item chosen.

////////////////////////////////////////////////////////////////////////////////
// GUI callbacks.

static void GammaScrollUpdate(	// Returns nothing.
	RScrollBar*	psb);					// Scrollbar that got updated.

static void DifficultyScrollUpdate(	// Returns nothing.
	RScrollBar*	psb);						// Scrollbar that got updated.

static void VolumesScrollUpdate(	// Returns nothing.
	RScrollBar*	psb);					// Scrollbar that got updated.

static void MouseSensitivityScrollUpdate(	// Returns nothing.
	RScrollBar* psb);								// Scrollbar that got updated.

static void RotationScrollUpdateDouble(	// Returns nothing.
	RScrollBar* psb);								// Scrollbar that got updated.

static void RotationScrollUpdateShort(		// Returns nothing.
	RScrollBar* psb);								// Scrollbar that got updated.

static void PainFrequencyScrollUpdate( // Returns nothing.
	RScrollBar* psb);					// Scrollbar that got updated.

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

CMenuSettings	g_MenuSettings;

// Controls.
static RScrollBar*	ms_psbGamma					= NULL;

static REdit*			ms_peditConnect	= NULL;
static REdit*			ms_peditHostName	= NULL;
static REdit*			ms_peditName	= NULL;
static RTxt*			ms_ptxtColor	= NULL;
static RTxt*			ms_ptxtProto  = NULL;
static RTxt*			ms_ptxtLanguage	= NULL;

static RMultiBtn*		ms_pmbCheckBox	= NULL;

static RScrollBar*	ms_psbDifficulty			= NULL;

static RScrollBar*	ms_psbMouseSensitivityX	= NULL;
static RScrollBar*	ms_psbMouseSensitivityY	= NULL;

static RTxt*			ms_ptxtBandwidth	= NULL;

static SampleMaster::SoundInstance		ms_siLastSamplePlayed	= 0;	// Last sample played.

static SampleMasterID*	ms_apsmidVolumeTesters[SampleMaster::MAX_NUM_SOUND_CATEGORIES]	=
	{
	&g_smidEmptyWeapon,		// Unspecified
	&g_smidMusicTester,		// BackgroundMusic
	&g_smidShotgun,			// Weapon
	&g_smidLoadedWeapon,		// UserFeedBack
	&g_smidRocketExplode,	// Destruction
	&g_smidAmbientTester,	// Ambient
	&g_smidDemonBleed,		// Demon.
	&g_smidSteveScumbag,		// Voices.
	&g_smidMikeAhh,			// Pain
	&g_smidWrithing2,			// Suffering
	};

static char*			ms_apszDifficultyDescriptions[]	=
	{
	"Easy",
	"Easy",
	"Easy",
	"Medium",
	"Medium",
	"Medium",
	"Hard",
	"Hard",
	"Hard",
	"Hard",
	"Masochist",
	};

char levelNames[22][256];
char challengeNames[10][256];
// Main menu
extern Menu	menuMain =
	{
	MAIN_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_SM,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_SM,				// menu header y offset
		MENU_ITEM_X_SM,				// menu items x offset
		MENU_ITEM_Y_SM,				// menu items y offset
		MENU_ITEM_SPACE_Y_SM,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_SM,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor, sStartIndex, sNumEntries
		MENU_BG_SM, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszMainMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		MainMenuInit,	// Called before menu is initialized.
		MainMenuChoice	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
#if defined(SPAWN)
		{	// pszText,						sEnabled,	pmenu,				pgui
			{ g_pszStartMultiplayerMenu_Join,TRUE,	&menuJoinMulti,	NULL,			},
			{ g_pszMainMenu_Options,	TRUE,			&menuOptions,		NULL,			},
			{ g_pszMainMenu_Exit,		TRUE,			&menuVerifyExit,	NULL,			},
			NULL							// Terminates list.
		},
#else
		{	// pszText,						sEnabled,	pmenu,				pgui
			{ g_pszMainMenu_Start,		TRUE,			&menuStart/*Single*/,			NULL,			},
			{ g_pszMainMenu_Options,	TRUE,			&menuOptions,		NULL,			},

			#ifndef EDITOR_REMOVED
			{ g_pszMainMenu_Editor,		TRUE,			NULL,					NULL,			},
			#endif

	#if defined(DEMO)
			{ g_pszMainMenu_Buy,			TRUE,			NULL,					NULL,			},
	#endif
			{ g_pszMainMenu_Exit,		TRUE,			&menuVerifyExit,	NULL,			},
			NULL							// Terminates list.
		},
#endif
	};

// Verify exit menu
extern Menu	menuVerifyExit =
	{
	VERIFY_EXIT_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_SM,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_SM,				// menu header y offset
		MENU_ITEM_X_SM,				// menu items x offset
		MENU_ITEM_Y_SM,				// menu items y offset
		MENU_ITEM_SPACE_Y_SM,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_SM,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_SM, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,			// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),


	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszVerifyExitMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		NULL,							// Called before menu is initialized.
		VerifyExitMenuChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,				sEnabled,	pmenu,		pgui
			{ g_pszVerifyExitMenu_Yes,	TRUE,			NULL,			NULL,		},
			{ g_pszVerifyExitMenu_No,	TRUE,			&menuMain,	NULL,		},
			NULL							// Terminates list.
		},
	};

// Verify exit menu
extern Menu	g_menuVerifyQuitGame =
	{
	VERIFY_QUIT_GAME_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,			// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),


	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszVerifyQuitMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		NULL,							// Called before menu is initialized.
		Play_VerifyQuitMenuChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,						sEnabled,	pmenu,		pgui
			{ g_pszVerifyQuitMenu_Yes,	TRUE,			NULL,			NULL,		},
			{ g_pszVerifyQuitMenu_No,	TRUE,			NULL,			NULL,		},
			NULL							// Terminates list.
		},
	};

// Client game menu
extern Menu	menuClientGame =
	{
	CLIENT_GAME_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_SM,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_SM,				// menu header y offset
		MENU_ITEM_X_SM,				// menu items x offset
		MENU_ITEM_Y_SM,				// menu items y offset
		MENU_ITEM_SPACE_Y_SM,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_SM,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_SM, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,			// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszGameMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		NULL,							// Called before menu is initialized.
		Play_VerifyQuitMenuChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		0,		// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,		sEnabled,	pmenu,						pgui
			{ g_pszGameMenu_Continue,	TRUE,			NULL,							NULL, },
			{ g_pszGameMenu_Save,		TRUE,			NULL,							NULL, },
			{ g_pszGameMenu_Options,	TRUE,			&menuOptions,				NULL, },
			{ g_pszGameMenu_Quit,		TRUE,			NULL,							NULL, },
			NULL							// Terminates list.
		},
	};

// Editor menu
extern Menu	menuEditor =
	{
	EDITOR_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_SM,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_SM,				// menu header y offset
		MENU_ITEM_X_SM,				// menu items x offset
		MENU_ITEM_Y_SM,				// menu items y offset
		MENU_ITEM_SPACE_Y_SM,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_SM,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_SM, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,			// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszEditorMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		NULL,						// Called before menu is initialized.
		EditorMenuChoice,		// Called when item is chosen.
		},

	// Menu auto items.
		{		// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		0,		// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,							sEnabled,	pmenu,					pgui
			{ g_pszEditorMenu_Continue,	TRUE,			NULL,						NULL, },
			{ g_pszEditorMenu_Options,		TRUE,			&menuOptions,			NULL, },
			{ g_pszEditorMenu_Quit,			TRUE,			NULL,						NULL, },
			NULL							// Terminates list.
		},
	};

// Options menu.
extern Menu	menuOptions =
	{
	OPTIONS_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,			// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszOptionsMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		OptionsInit,	// Called before menu is initialized.
		OptionsChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,								sEnabled,	pmenu,					pgui
#if 0
			{ g_pszOptionsMenu_Video,			TRUE,			&menuVideoOptions,	NULL,	},
#endif
			{ g_pszOptionsMenu_Audio,			TRUE,			&menuAudioOptions,	NULL,	},
			{ g_pszOptionsMenu_Controls,		TRUE,			&menuControls,			NULL,	},
            #ifndef MULTIPLAYER_REMOVED
			{ g_pszOptionsMenu_Multiplayer,	TRUE,			&menuMultiOptions,	NULL,	},
            #endif
			{ g_pszOptionsMenu_Performance,	TRUE,			&menuFeatures,			NULL,	},
			{ g_pszOptionsMenu_Difficulty,	TRUE,			&menuPlayOptions,		NULL,	},
			{ g_pszGameMenu_Title,			TRUE,			&menuGameOptions,		NULL,	},
			{ "",										FALSE,		NULL,						NULL,	},
			NULL							// Terminates list.
		},
	};

// Game options menu.
extern Menu	menuGameOptions =
	{
	OPTIONS_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,			// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszGameMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		GameOptionsInit,	// Called before menu is initialized.
		GameOptionsChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,								sEnabled,	pmenu,					pgui
			{ g_pszMultiplayerSetupMenu_Color,			TRUE,			NULL,				NULL, },
			{ g_pszOptionsMenu_Crosshair,	TRUE,			NULL,		NULL,	},
			#ifdef KID_FRIENDLY_OPTION
			// Please note that this should always be the last option in the list,
			// otherwise we can't remove it.
			{ g_pszOptionsMenu_KidMode,		TRUE,		NULL,		NULL, },
			#endif
			{ "",										FALSE,		NULL,						NULL,	},
			NULL							// Terminates list.
		},
	};

// Options menu.
extern Menu	menuPlayOptions =
	{
	PLAYOPTIONS_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,			// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszDifficultyMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		PlayOptionsInit,		// Called before menu is initialized.
		PlayOptionsChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,										sEnabled,	pmenu,					pgui
			{ g_pszDifficultyMenu_SetDifficulty,	TRUE,			NULL,						NULL,				},
			{ "",												FALSE,		NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};

// Postal music organ:
extern Menu menuOrgan = 
	{
	ORGAN_MENU_ID,
	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,			// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszOrganMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		NULL,					// Called before menu is initialized.
		Organ_MenuChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,											sEnabled,	pmenu,	pgui
			{ "",													FALSE,		NULL,		NULL,				},
			{ g_pszOrganMenu_SpecialKeysHeading,		FALSE,		NULL,		NULL,				},
			{ g_pszOrganMenu_NumericKeysFunction,		FALSE,		NULL,		NULL,				},
			{ g_pszOrganMenu_AlphaKeysFunction,			FALSE,		NULL,		NULL,				},
			{ g_pszOrganMenu_TabKeyFunction,				FALSE,		NULL,		NULL,				},
			{ "",													FALSE,		NULL,		NULL,				},
			{ g_pszOrganMenu_Exit,							TRUE,			NULL,		NULL,				},
			NULL							// Terminates list.
		},
	};

// Audio Options menu.
extern Menu	menuAudioOptions =
	{
	AUDIO_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszAudioMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		AudioOptionsInit,		// Called before menu is initialized.
		AudioOptionsChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,				sEnabled,	pmenu,					pgui
			{ g_pszAudioMenu_Mixer,				TRUE,			&menuVolumes,			NULL,				},
			{ g_pszAudioMenu_SoundTest,		TRUE,			&menuOrgan,				NULL,				},
			{ g_pszAudioMenu_Language,		TRUE,			NULL,					NULL,				},
			{ g_pszAudioMenu_PainFrequency, TRUE, NULL, NULL, },
			{ "",										FALSE,		NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};


// Video Options menu.
extern Menu	menuVideoOptions =
	{
	VIDEO_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_SM,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_SM,				// menu header y offset
		MENU_ITEM_X_SM,				// menu items x offset
		MENU_ITEM_Y_SM,				// menu items y offset
		MENU_ITEM_SPACE_Y_SM,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_SM,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_SM, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszVideoMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		VideoOptionsInit,	// Called before menu is initialized.
		VideoOptionsChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,					sEnabled,	pmenu,					pgui
			{ g_pszVideoMenu_Gamma,	TRUE,			NULL,						NULL,				},
			{ "",							FALSE,		NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};

// Controls menu.
extern Menu	menuControls =
	{
	CONTROLS_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_LG,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_LG,				// menu header y offset
		MENU_ITEM_X_LG,				// menu items x offset
		MENU_ITEM_Y_LG,				// menu items y offset
		MENU_ITEM_SPACE_Y_LG,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_LG,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_LG, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszControlsMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		ControlsInit,		// Called before menu is initialized.
		ControlsChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,												sEnabled,	pmenu,					pgui
			{ g_pszControlsMenu_KeyboardSetup,				TRUE,			&menuKeyboard,			NULL,				},
			{ g_pszControlsMenu_MouseSetup,					TRUE,			&menuMouse,				NULL,				},
#if defined(ALLOW_JOYSTICK)
			{ g_pszControlsMenu_JoystickSetup,				TRUE,			&menuJoystick,			NULL,				},
#endif // defined(ALLOW_JOYSTICK)
			{ g_pszControlsMenu_TurningSpeeds,				TRUE,			&menuRotation,			NULL,				},
#if defined(ALLOW_JOYSTICK)
			{ g_pszControlsMenu_UseJoystick,					TRUE,			NULL,						NULL,				},
#endif // defined(ALLOW_JOYSTICK)
			{ g_pszControlsMenu_UseMouse,						TRUE,			NULL,						NULL,				},
			{ g_pszControlsMenu_ClassicMouse,                               TRUE,               NULL,               NULL                        },
			{ g_pszControlsMenu_HorizMouseSensitivity,	TRUE,			NULL,						NULL,				},
			{ g_pszControlsMenu_VertMouseSensitivity,		TRUE,			NULL,						NULL,				},
			{ "",														FALSE,		NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};

// Keyboard menu.
extern Menu	menuKeyboard =
	{
	KEYBOARD_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_LG,	// Pos and dimensions.
		15,					// Offset from left edge for menu header.
								// Negative indicates offset from center.
		12,					// Offset from top edge for menu header.
								// Negative indicates offset from center.
		72,					// Offset from left edge for menu items.
								// Negative indicates offset from center.
		40,					// Offset from top edge for menu items.
								// Negative indicates offset from center.
		-5,						// Space between menu items.
		5,						// Space between indicator and menu items horizontally.
		-10,					// X position menu items should not pass w/i Menu.
								// Less than 1, indicates offset from right edge. 
		-20,					// Y position menu items should not pass w/i Menu.
								// Less than 1, indicates offset from right edge. 
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_LG, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszKeyboardSetupMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT_SMALLER,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		KeyInit,		// Called before menu is initialized.
		KeyChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
	// Note that in the old ways we had to reserve as many of these
	// as there were items to be in this menu, but now, since this array
	// is not open ended, we don't.  But, if it ever ends up open ended
	// again, this will have to be updated.  I included enough for the 
	// current settings, but more will have to be added, if we ever change
	// back and add more input functions.
		{	// pszText,				sEnabled,	pmenu,					pgui
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};

// Mouse menu.
extern Menu	menuMouse =
	{
	MOUSE_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_LG,	// Pos and dimensions.
		15,					// Offset from left edge for menu header.
								// Negative indicates offset from center.
		12,					// Offset from top edge for menu header.
								// Negative indicates offset from center.
		72,					// Offset from left edge for menu items.
								// Negative indicates offset from center.
		40,					// Offset from top edge for menu items.
								// Negative indicates offset from center.
		-5,						// Space between menu items.
		5,						// Space between indicator and menu items horizontally.
		-10,					// X position menu items should not pass w/i Menu.
								// Less than 1, indicates offset from right edge. 
		-20,					// Y position menu items should not pass w/i Menu.
								// Less than 1, indicates offset from right edge. 
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_LG, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszMouseSetupMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT_SMALLER,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		MouseInit,		// Called before menu is initialized.
		MouseChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
	// Note that in the old ways we had to reserve as many of these
	// as there were items to be in this menu, but now, since this array
	// is not open ended, we don't.  But, if it ever ends up open ended
	// again, this will have to be updated.  I included enough for the 
	// current settings, but more will have to be added, if we ever change
	// back and add more input functions.
		{	// pszText,				sEnabled,	pmenu,					pgui
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};

// Joystick menu.
extern Menu	menuJoystick =
	{
	JOYSTICK_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_LG,	// Pos and dimensions.
		15,					// Offset from left edge for menu header.
								// Negative indicates offset from center.
		12,					// Offset from top edge for menu header.
								// Negative indicates offset from center.
		72,					// Offset from left edge for menu items.
								// Negative indicates offset from center.
		40,					// Offset from top edge for menu items.
								// Negative indicates offset from center.
		-5,						// Space between menu items.
		5,						// Space between indicator and menu items horizontally.
		-10,					// X position menu items should not pass w/i Menu.
								// Less than 1, indicates offset from right edge. 
		-20,					// Y position menu items should not pass w/i Menu.
								// Less than 1, indicates offset from right edge. 
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_LG, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszJoystickSetupMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT_SMALLER,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		JoyInit,		// Called before menu is initialized.
		JoyChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
	// Note that in the old ways we had to reserve as many of these
	// as there were items to be in this menu, but now, since this array
	// is not open ended, we don't.  But, if it ever ends up open ended
	// again, this will have to be updated.  I included enough for the 
	// current settings, but more will have to be added, if we ever change
	// back and add more input functions.
		{	// pszText,				sEnabled,	pmenu,					pgui
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};


// Level select.
extern Menu	menuLoadLevel =
	{
	LOAD_LEVEL_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_LG,	// Pos and dimensions.
		15,					// Offset from left edge for menu header.
								// Negative indicates offset from center.
		12,					// Offset from top edge for menu header.
								// Negative indicates offset from center.
		72,					// Offset from left edge for menu items.
								// Negative indicates offset from center.
		56,					// Offset from top edge for menu items.
								// Negative indicates offset from center.
		-5,						// Space between menu items.
		5,						// Space between indicator and menu items horizontally.
		-10,					// X position menu items should not pass w/i Menu.
								// Less than 1, indicates offset from right edge. 
		-90,					// Y position menu items should not pass w/i Menu.
								// Less than 1, indicates offset from right edge. 
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_LG, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartSinglePlayerMenu_LoadLevel,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT_SMALLER,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		LoadLevelInit,		// Called before menu is initialized.
		LoadLevelChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},

	// Menu items.
	// Note that in the old ways we had to reserve as many of these
	// as there were items to be in this menu, but now, since this array
	// is not open ended, we don't.  But, if it ever ends up open ended
	// again, this will have to be updated.  I included enough for the 
	// current settings, but more will have to be added, if we ever change
	// back and add more input functions.

		{	// pszText,				sEnabled,	pmenu,					pgui
			
			// original game
			{ levelNames[0], TRUE, NULL, NULL, },
			{ levelNames[1], TRUE, NULL, NULL, },
			{ levelNames[2], TRUE, NULL, NULL, },
			{ levelNames[3], TRUE, NULL, NULL, },
			{ levelNames[4], TRUE, NULL, NULL, },
			{ levelNames[5], TRUE, NULL, NULL, },
			{ levelNames[6], TRUE, NULL, NULL, },
			{ levelNames[7], TRUE, NULL, NULL, },
			{ levelNames[8], TRUE, NULL, NULL, },
			{ levelNames[9], TRUE, NULL, NULL, },
			{ levelNames[10], TRUE, NULL, NULL, },
			{ levelNames[11], TRUE, NULL, NULL, },
			{ levelNames[12], TRUE, NULL, NULL, },
			{ levelNames[13], TRUE, NULL, NULL, },
			{ levelNames[14], TRUE, NULL, NULL, },
			{ levelNames[15], TRUE, NULL, NULL, },
			
			// SD and Plus
			{ levelNames[16], TRUE, NULL, NULL, },
			{ levelNames[17], TRUE, NULL, NULL, },
			{ levelNames[18], TRUE, NULL, NULL, },
			{ levelNames[19], TRUE, NULL, NULL, },
			{ levelNames[20], TRUE, NULL, NULL, },
			{ levelNames[21], TRUE, NULL, NULL, },
			
			{ "", FALSE, NULL, NULL, }, // This needs to be at the end of the list or ESC will load the last level
			NULL							// Terminates list.
		},
	};

// Features menu.
extern Menu	menuFeatures =
	{
	FEATURES_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-80,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszPerformanceMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		FeaturesInit,		// Called before menu is initialized.
		FeaturesChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,										sEnabled,	pmenu,					pgui
			{ g_pszPerformanceMenu_Transparency,	TRUE,			NULL,						NULL,				},
			{ g_pszPerformanceMenu_3dLighting,		TRUE,			NULL,						NULL,				},
			{ g_pszPerformanceMenu_Particles,		TRUE,			NULL,						NULL,				},
			{ g_pszPerformanceMenu_DynamicVolume,	TRUE,			NULL,						NULL,				},
			{ g_pszPerformanceMenu_AmbientSounds,	TRUE,			NULL,						NULL,				},
			{ "",												FALSE,		NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};

// Rotation menu.
extern Menu	menuRotation =
	{
	ROTATION_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_LG,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_LG,				// menu header y offset
		MENU_ITEM_X_LG,				// menu items x offset
		MENU_ITEM_Y_LG,				// menu items y offset
		MENU_ITEM_SPACE_Y_LG,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_LG,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_LG, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszRotationSetupMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		RotationInit,		// Called before menu is initialized.
		RotationChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,											sEnabled,	pmenu,					pgui
			{ g_pszRotationSetupMenu_RunningSlow,		TRUE,			NULL,						NULL,				},
			{ g_pszRotationSetupMenu_RunningFast,		TRUE,			NULL,						NULL,				},
			{ g_pszRotationSetupMenu_StandingSlow,		TRUE,			NULL,						NULL,				},
			{ g_pszRotationSetupMenu_StandingFast,		TRUE,			NULL,						NULL,				},
			{ g_pszRotationSetupMenu_TapDegrees,		TRUE,			NULL,						NULL,				},
			{ g_pszRotationSetupMenu_RestoreDefaults,	TRUE,			NULL,						NULL,				},
			{ "",													FALSE,		NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};

// Volumes menu.
extern Menu	menuVolumes =
	{
	VOLUME_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		1, //MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszVolumesMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT_SMALLER,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		VolumesInit,	// Called before menu is initialized.
		VolumesChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
	// Note that in the old ways we had to reserve as many of these
	// as there were items to be in this menu, but now, since this array
	// is not open ended, we don't.  But, if it ever ends up open ended
	// again, this will have to be updated.  I included enough for the 
	// current settings, but more will have to be added, if we ever change
	// back and add more volume categories.
		{	// pszText,				sEnabled,	pmenu,					pgui
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			{ NULL,					TRUE,			NULL,						NULL,				},
			NULL							// Terminates list.
		},
	};

// Start menu.
extern Menu	menuStart =
	{
	START_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartGameMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		StartGameInit,	// Called before menu is initialized.
		StartGameMenu,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,											sEnabled,	pmenu,				pgui
			{ g_pszStartGameMenu_SinglePlayer,			TRUE,			&menuStartSingle,	NULL,	},
            //#ifndef MULTIPLAYER_REMOVED
			{ g_pszStartGameMenu_Multiplayer,			TRUE,			&menuStartMulti,	NULL, },
            //#endif
			{ g_pszStartGameMenu_Demo,						TRUE,			&menuStartDemo,	NULL,	},
			{ "",													FALSE,		NULL,					NULL, },
			NULL							// Terminates list.
		},
	};

// Single player start menu.
extern Menu	menuStartSingle =
	{
	START_SINGLE_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartSinglePlayerMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		StartSingleInit,	// Called before menu is initialized.
		StartSingleMenu,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,											sEnabled,	pmenu,				pgui
			{ g_pszStartSinglePlayerMenu_NewCampaign,			TRUE,			&menuNewCampaign,					NULL,	},
			{ g_pszStartSinglePlayerMenu_LoadGame,		TRUE,			NULL,					NULL, },
#ifndef LOADLEVEL_REMOVED
			{ g_pszStartSinglePlayerMenu_LoadLevel,	TRUE,			&menuLoadLevel,					NULL,	},
#endif
			{ "CHALLENGES",	TRUE,			&menuChallenge,					NULL,	},
			{ "",													FALSE,		NULL,					NULL, },
			NULL							// Terminates list.
		},
	};
	
// Single player new campaign menu.
extern Menu	menuNewCampaign =
	{
	START_SINGLE_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_LG,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_LG,				// menu header y offset
		MENU_ITEM_X_LG,				// menu items x offset
		MENU_ITEM_Y_LG,				// menu items y offset
		MENU_ITEM_SPACE_Y_LG,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_LG,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_LG, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartSinglePlayerMenu_NewCampaign,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		NewCampaignInit,	// Called before menu is initialized.
		NewCampaignMenu,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,											sEnabled,	pmenu,				pgui
			{ g_pszStartSinglePlayerMenu_New,			TRUE,			NULL,					NULL,	},
#if defined(START_MENU_ADDON_ITEM)
			{ g_pszStartSinglePlayerMenu_AddOn,			TRUE,			NULL,					NULL,	},	
#if TARGET == POSTAL_2015
			{ g_pszStartSinglePlayerMenu_AddOn2,		TRUE,			NULL,					NULL,	},
			{ g_pszStartSinglePlayerMenu_AllLevels,		TRUE,			NULL,					NULL,	},
#endif
#endif
			{ "",													FALSE,		NULL,					NULL, },
			NULL							// Terminates list.
		},
	};
	
// Single player start menu.
extern Menu	menuChallenge =
	{
	CHALLENGE_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartChallengeMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		ChallengeInit,		// Called before menu is initialized.
		ChallengeChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,					sEnabled,		pmenu,		pgui
			{ g_pszStartChallengeMenu_Gauntlet,		TRUE,			NULL,			NULL,	},
			{ g_pszStartChallengeMenu_Timed,			TRUE,			&menuChallengeTimed,			NULL,	},
			{ g_pszStartChallengeMenu_Goal,			TRUE,			&menuChallengeGoal,			NULL,	},
			{ g_pszStartChallengeMenu_Flag,			TRUE,			&menuChallengeFlag,			NULL,	},
			{ g_pszStartChallengeMenu_CheckPoint,	TRUE,			&menuChallengeCheckpoint,			NULL,	},
			{ "",												FALSE,		NULL,			NULL, },
			NULL							// Terminates list.
		},
	};

extern Menu menuChallengeTimed = 
{
	CHALLENGE_TIMED_MENU_ID,

	// Position info.
	{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
	},

	// Background info.
	{	// pszFile, u32BackColor
		MENU_BG_MD,
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
	},

	// GUI settings.
	{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
	},

	// Flags.
	(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
	{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartChallengeMenu_Timed,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
	},

	// Font info.
	{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
	},

	// Menu indicator.
	{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
	},

	// Menu callbacks.
	{	// fnInit, fnChoice,
		ChallengeInit,		// Called before menu is initialized.
		ChallengeTimedChoice,	// Called when item is chosen.
	},

	// Menu auto items.
	{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
				-1,	// Menu item (index in ami[]) chosen on cancel.
					// Negative indicates distance from number of items
					// (e.g., -1 is the last item).
	},

	// Menu items.
	{	// pszText,					sEnabled,		pmenu,		pgui
		{ challengeNames[0],		TRUE,			NULL,			NULL, },
		{ challengeNames[1],			TRUE,			NULL,			NULL, },
		{ challengeNames[2],			TRUE,			NULL,			NULL, },
		{ challengeNames[3],	TRUE,			NULL,			NULL, },
		{ challengeNames[4],	TRUE,			NULL,			NULL, },
		{ "",												FALSE,		NULL,			NULL, },
		NULL							// Terminates list.
	},
};

extern Menu menuChallengeGoal =
{
	CHALLENGE_GOAL_MENU_ID,

	// Position info.
	{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
	},

	// Background info.
	{	// pszFile, u32BackColor
		MENU_BG_MD,
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
	},

	// GUI settings.
	{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
	},

	// Flags.
	(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
	{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartChallengeMenu_Goal,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
	},

	// Font info.
	{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
	},

	// Menu indicator.
	{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
	},

	// Menu callbacks.
	{	// fnInit, fnChoice,
		ChallengeInit,		// Called before menu is initialized.
		ChallengeGoalChoice,	// Called when item is chosen.
	},

	// Menu auto items.
	{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
				-1,	// Menu item (index in ami[]) chosen on cancel.
					// Negative indicates distance from number of items
					// (e.g., -1 is the last item).
	},

	// Menu items.
	{	// pszText,					sEnabled,		pmenu,		pgui
		{ challengeNames[0],		TRUE,			NULL,			NULL, },
		{ challengeNames[1],			TRUE,			NULL,			NULL, },
		{ challengeNames[2],			TRUE,			NULL,			NULL, },
		{ challengeNames[3],			TRUE,			NULL,			NULL, },
		{ challengeNames[4],	TRUE,			NULL,			NULL, },
		{ challengeNames[5],	TRUE,			NULL,			NULL, },
		{ "",												FALSE,		NULL,			NULL, },
		NULL							// Terminates list.
	},
};

extern Menu menuChallengeFlag =
{
	CHALLENGE_FLAG_MENU_ID,

	// Position info.
	{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
	},

	// Background info.
	{	// pszFile, u32BackColor
		MENU_BG_MD,
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
	},

	// GUI settings.
	{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
	},

	// Flags.
	(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
	{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartChallengeMenu_Flag,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
	},

	// Font info.
	{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
	},

	// Menu indicator.
	{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
	},

	// Menu callbacks.
	{	// fnInit, fnChoice,
		ChallengeInit,		// Called before menu is initialized.
		ChallengeFlagChoice,	// Called when item is chosen.
	},

	// Menu auto items.
	{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
				-1,	// Menu item (index in ami[]) chosen on cancel.
					// Negative indicates distance from number of items
					// (e.g., -1 is the last item).
	},

	// Menu items.
	{	// pszText,					sEnabled,		pmenu,		pgui
		{ challengeNames[0],		TRUE,			NULL,			NULL, },
		{ challengeNames[1],			TRUE,			NULL,			NULL, },
		{ challengeNames[2],			TRUE,			NULL,			NULL, },
		{ challengeNames[3],			TRUE,			NULL,			NULL, },
		{ challengeNames[4],	TRUE,			NULL,			NULL, },
		{ challengeNames[5],	TRUE,			NULL,			NULL, },
		{ "",												FALSE,		NULL,			NULL, },
		NULL							// Terminates list.
	},
};

extern Menu menuChallengeCheckpoint =
{
	CHALLENGE_CHECKPOINT_MENU_ID,

	// Position info.
	{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
	},

	// Background info.
	{	// pszFile, u32BackColor
		MENU_BG_MD,
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
	},

	// GUI settings.
	{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
	},

	// Flags.
	(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
	{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartChallengeMenu_CheckPoint,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
	},

	// Font info.
	{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
	},

	// Menu indicator.
	{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
	},

	// Menu callbacks.
	{	// fnInit, fnChoice,
		ChallengeInit,		// Called before menu is initialized.
		ChallengeCheckpointChoice,	// Called when item is chosen.
	},

	// Menu auto items.
	{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
				-1,	// Menu item (index in ami[]) chosen on cancel.
					// Negative indicates distance from number of items
					// (e.g., -1 is the last item).
	},

	// Menu items.
	{	// pszText,					sEnabled,		pmenu,		pgui
		{ challengeNames[0],		TRUE,			NULL,			NULL, },
		{ challengeNames[1],			TRUE,			NULL,			NULL, },
		{ challengeNames[2],			TRUE,			NULL,			NULL, },
		{ challengeNames[3],			TRUE,			NULL,			NULL, },
		{ challengeNames[4],	TRUE,			NULL,			NULL, },
		{ "",												FALSE,		NULL,			NULL, },
		NULL							// Terminates list.
	},
};

// Multiplayer start menu.
extern Menu	menuStartMulti =
	{
	START_MULTI_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD,
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartMultiplayerMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		StartMultiInit,	// Called before menu is initialized.
		StartMultiMenu,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,										sEnabled,	pmenu,					pgui
			{ g_pszStartMultiplayerMenu_Join,		TRUE,			&menuJoinMulti,		NULL, },
			{ g_pszStartMultiplayerMenu_Host,		TRUE,			&menuHostMulti,		NULL,	},
			{ g_pszStartMultiplayerMenu_Options,	TRUE,			&menuMultiOptions,	NULL,	},
			{ "",												FALSE,		NULL,						NULL, },
			NULL							// Terminates list.
		},
	};

// Join Multiplayer menu.
extern Menu	menuJoinMulti =
	{
	JOIN_MULTI_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_LG,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_LG,				// menu header y offset
		MENU_ITEM_X_LG,				// menu items x offset
		MENU_ITEM_Y_LG,				// menu items y offset
		MENU_ITEM_SPACE_Y_LG,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_LG,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_LG, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszJoinGameMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		JoinMultiInit,	// Called before menu is initialized.
		JoinMultiMenu,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,								sEnabled,	pmenu,					pgui
			{ g_pszJoinGameMenu_Browse,		TRUE,			NULL,						NULL, },
			{ g_pszJoinGameMenu_ConnectTo,	TRUE,			NULL,						NULL,	},
			{ "",										FALSE,		NULL,						NULL, },
			NULL							// Terminates list.
		},
	};

// Join Multiplayer menu.
extern Menu	menuHostMulti =
	{
	HOST_MULTI_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszHostGameMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		HostMultiInit,	// Called before menu is initialized.
		HostMultiMenu,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,						sEnabled,	pmenu,					pgui
			{ g_pszHostGameMenu_Start,	TRUE,			NULL,						NULL, },
			{ "",								FALSE,		NULL,						NULL, },
			NULL							// Terminates list.
		},
	};

// Single player start menu.
extern Menu	menuStartDemo =
	{
	START_DEMO_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_SM,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_SM,				// menu header y offset
		MENU_ITEM_X_SM,				// menu items x offset
		MENU_ITEM_Y_SM,				// menu items y offset
		MENU_ITEM_SPACE_Y_SM,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_SM,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_SM, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszStartDemoMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		StartDemoInit,	// Called before menu is initialized.
		StartDemoMenu,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,							sEnabled,	pmenu,		pgui
//			{ g_pszStartDemoMenu_Browse,	TRUE,			NULL,			NULL,	},
//			{ g_pszStartDemoMenu_Play,		TRUE,			NULL,			NULL,	},
//			{ g_pszStartDemoMenu_Record,	TRUE,			NULL,			NULL,	},
			{ g_pszStartDemoMenu_ConSite,	TRUE,			NULL,			NULL,	},
			{ g_pszStartDemoMenu_Home,		TRUE,			NULL,			NULL,	},
			{ g_pszStartDemoMenu_Bridge,	TRUE,			NULL,			NULL,	},
			{ "",									FALSE,		NULL,			NULL, },
			NULL							// Terminates list.
		},
	};

// Multiplayer options menu.
extern Menu	menuMultiOptions =
	{
	MULTIPLAYER_OPTIONS_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-60,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,		// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter | MenuColumnizeGuis),

	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		g_pszMultiplayerSetupMenu_Title,
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		MultiOptionsInit,		// Called before menu is initialized.
		MultiOptionsChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
		{	// pszText,				sEnabled,	pmenu,			pgui
			{ g_pszMultiplayerSetupMenu_Name,			TRUE,			NULL,				NULL,	},
			{ g_pszMultiplayerSetupMenu_Protocol,		TRUE,			NULL,				NULL,	},
			{ g_pszMultiplayerSetupMenu_Connection,	TRUE,			NULL,				NULL,	},
			{ "",													FALSE,		NULL,				NULL,	},
			NULL							// Terminates list.
		},
	};

////////////////////////////////////////////////////////////////////////////////
// Menu callbacks.

////////////////////////////////////////////////////////////////////////////////
//
// Called when main menu is initialized or killed.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t MainMenuInit(		// Returns 0 on succes, non-zero to cancel menu.
	Menu*	pmenuCurrent,			// In:  Menu being init'ed or killed.
	int16_t sInit)					// In:  TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	Game_InitMainMenu(sInit);

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when item is chosen from main menu
//
////////////////////////////////////////////////////////////////////////////////
static bool MainMenuChoice(	// Returns true to accept, false to deny choice.
	Menu*	/*pmenuCurrent*/,		// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	switch (sMenuItem)
		{
		case 2:
			#if defined(SPAWN) || defined(EDITOR_REMOVED)
				// Exit
			#else
				// Editor
				Game_StartEditor();
			#endif
			break;

		case 3:
			#if defined(DEMO)
				// Buy
				Game_Buy();
			#else
				// (some other choice we don't care about)
			#endif
			break;
		}

	return bAcceptChoice;
	}

static bool VerifyExitMenuChoice(	// Returns true to accept, false to deny choice.
	Menu*	/*pmenuCurrent*/,		// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	switch (sMenuItem)
		{
		case 0:	// Yes
			// Set quit status to something other than 1 (which is what RSPiX Blue sets it to
			// if a system-specific quit is used) so we can later detect wether we should immediately
			// quit the app or put up the credits first.
			rspSetQuitStatus(10);
			break;
		case 1:	// No
			break;
		}

	return bAcceptChoice;
	}

static bool ClientGameMenuChoice(	// Returns true to accept, false to deny choice.
	Menu*	/*pmenuCurrent*/,		// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	switch (sMenuItem)
		{
		case 0:	// Continue.
			StopMenu();
			break;
		case 1:	// Options.
			break;
		case 2:	// Quit.
			break;
		}

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the Start Game menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t StartGameInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	//// Be sure the demo option reflects INI setting.
	//if (g_GameSettings.m_sCanRecordDemos == FALSE)
		//{
		//menuStart.ami[2].pmenu	= NULL;
		//}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called by the menu API when an item is chosen in menuStart.
//
////////////////////////////////////////////////////////////////////////////////
static bool StartGameMenu(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	switch (sMenuItem)
		{
        #ifndef MULTIPLAYER_REMOVED
		case 1:
			// This is necessary to give the game a chance to inform the
			// player in case multiplayer mode is disabled.  The return
			// value determines whether we accept this choice.
			bAcceptChoice = Game_StartMultiPlayerGame(sMenuItem);
			break;

		case 2:
        #else
		case 1:
        #endif

			//// If we can't record...
			//if (g_GameSettings.m_sCanRecordDemos == FALSE)
				//{
				//// Start a demo in playback mode by simulating the menu choice "Play"
				//// from the demo menu.
				//Game_StartDemoGame(1);
				//}
			break;
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
// Called to init or kill the Challenge menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t ChallengeInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.
	
	//Fill Challenge Names into array.
	switch (pmenuCur->u32Id)
	{
		case 29:
			strcpy(challengeNames[0], GetChallengeTitle(10));
			strcpy(challengeNames[1], GetChallengeTitle(4));
			strcpy(challengeNames[2], GetChallengeTitle(20));
			strcpy(challengeNames[3], GetChallengeTitle(7));
			strcpy(challengeNames[4], GetChallengeTitle(9));
			break;
		case 30:
			strcpy(challengeNames[0], GetChallengeTitle(16));
			strcpy(challengeNames[1], GetChallengeTitle(14));
			strcpy(challengeNames[2], GetChallengeTitle(2));
			strcpy(challengeNames[3], GetChallengeTitle(19));
			strcpy(challengeNames[4], GetChallengeTitle(23));
			strcpy(challengeNames[5], GetChallengeTitle(21));
			break;
		case 31:
			strcpy(challengeNames[0], GetChallengeTitle(5));
			strcpy(challengeNames[1], GetChallengeTitle(18));
			strcpy(challengeNames[2], GetChallengeTitle(12));
			strcpy(challengeNames[3], GetChallengeTitle(13));
			strcpy(challengeNames[4], GetChallengeTitle(22));
			strcpy(challengeNames[5], GetChallengeTitle(15));
			break;
		case 32:
			strcpy(challengeNames[0], GetChallengeTitle(6));
			strcpy(challengeNames[1], GetChallengeTitle(8));
			strcpy(challengeNames[2], GetChallengeTitle(3));
			strcpy(challengeNames[3], GetChallengeTitle(17));
			strcpy(challengeNames[4], GetChallengeTitle(11));
			break;
		default:
			break;
	}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called by the menu API when an item is chosen in menuChallenge.
//
////////////////////////////////////////////////////////////////////////////////
static bool ChallengeChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.
	

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	// Let game module handle it
	if (sMenuItem == 0)
		Game_StartChallengeGame(0);

	return bAcceptChoice;
	}

static bool ChallengeTimedChoice(
	Menu* pmenuCurrent,
	int16_t sMenuItem)
	{
	//Assume Accepting
	bool bAcceptChoice = true;

	//Audible Feedback
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	//HANDLE SELECTION
	switch (sMenuItem)
	{
	case 0:
		Game_StartChallengeGame(10);
		break;
	case 1:
		Game_StartChallengeGame(4);
		break;
	case 2:
		Game_StartChallengeGame(20);
		break;
	case 3:
		Game_StartChallengeGame(7);
		break;
	case 4:
		Game_StartChallengeGame(9);
		break;
	default:
		break;
	}

	return bAcceptChoice;

	}

static bool ChallengeGoalChoice(
	Menu* pmenuCurrent,
	int16_t sMenuItem)
{
	//Assume Accepting
	bool bAcceptChoice = true;

	//Audible Feedback
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	//HANDLE SELECTION
	switch (sMenuItem)
	{
	case 0:
		Game_StartChallengeGame(16);
		break;
	case 1:
		Game_StartChallengeGame(14);
		break;
	case 2:
		Game_StartChallengeGame(2);
		break;
	case 3:
		Game_StartChallengeGame(19);
		break;
	case 4:
		Game_StartChallengeGame(23);
		break;
	case 5:
		Game_StartChallengeGame(21);
		break;
	default:
		break;
	}

	return bAcceptChoice;

}

static bool ChallengeFlagChoice(
	Menu* pmenuCurrent,
	int16_t sMenuItem)
{
	//Assume Accepting
	bool bAcceptChoice = true;

	//Audible Feedback
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	//HANDLE SELECTION
	switch (sMenuItem)
	{
	case 0:
		Game_StartChallengeGame(5);
		break;
	case 1:
		Game_StartChallengeGame(18);
		break;
	case 2:
		Game_StartChallengeGame(12);
		break;
	case 3:
		Game_StartChallengeGame(13);
		break;
	case 4:
		Game_StartChallengeGame(22);
		break;
	case 5:
		Game_StartChallengeGame(15);
		break;
	default:
		break;
	}

	return bAcceptChoice;

}

static bool ChallengeCheckpointChoice(
	Menu* pmenuCurrent,
	int16_t sMenuItem)
{
	//Assume Accepting
	bool bAcceptChoice = true;

	//Audible Feedback
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	//HANDLE SELECTION
	switch (sMenuItem)
	{
	case 0:
		Game_StartChallengeGame(6);
		break;
	case 1:
		Game_StartChallengeGame(8);
		break;
	case 2:
		Game_StartChallengeGame(3);
		break;
	case 3:
		Game_StartChallengeGame(17);
		break;
	case 4:
		Game_StartChallengeGame(11);
		break;
	default:
		break;
	}

	return bAcceptChoice;

}
////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the Start Single Player Game menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t StartSingleInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	return 0;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called by the menu API when an item is chosen in menuStartSingle.
//
////////////////////////////////////////////////////////////////////////////////
static bool StartSingleMenu(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	// Let game module handle it... Sorta.
	// Because of the new menu split the numbers don't match up.
	// Let's figure that out.
	switch (sMenuItem)
	{
		case 1:
			Game_StartSinglePlayerGame(6);
			break;
		#ifndef LOADLEVEL_REMOVED
		case 2:
			Game_StartSinglePlayerGame(5);
			break;
		case 3:
		#else
		case 2:
		#endif // LOADLEVEL_REMOVED
			//Game_StartSinglePlayerGame(7);
			break;
	}

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the New Campaign menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t NewCampaignInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	return 0;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called by the menu API when an item is chosen in menuNewCampaign.
//
////////////////////////////////////////////////////////////////////////////////
static bool NewCampaignMenu(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	// Let game module handle it
	Game_StartSinglePlayerGame(sMenuItem);

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the Start Multiplayer Game menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t StartMultiInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called by the menu API when an item is chosen in menuStartMulti.
//
////////////////////////////////////////////////////////////////////////////////
static bool StartMultiMenu(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	return bAcceptChoice;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill menuJoinMulti.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t JoinMultiInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

		if (rspGetResource(&g_resmgrShell, GUI_CONNECT_IP_FILE, &ms_peditConnect) == 0)
			{
			// Set the text from the INI setting.  Note that we are changing a resource!
			ms_peditConnect->m_sMaxText = sizeof(g_GameSettings.m_szServerName) - 1;
			ms_peditConnect->SetText("%s", g_GameSettings.m_szServerName);
			ms_peditConnect->Compose();

			// Let menu know about it.
			pmenuCur->ami[1].pgui	= ms_peditConnect;
			}
		else
			{
			TRACE("JoinMultiInit(): ms_presmgr->Get() failed.\n");
			sRes	= 1;
			}
		}
	else
		{
		// Release resources
		rspReleaseResource(&g_resmgrShell, &ms_peditConnect);

		// Clear menu's pointer.
		pmenuCur->ami[1].pgui	= NULL;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called by the menu API when an item is chosen in menuJoinMulti.
//
////////////////////////////////////////////////////////////////////////////////
static bool JoinMultiMenu(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	// If specified, get server name (BEFORE calling the game module!)
	if (ms_peditConnect != NULL)
		ms_peditConnect->GetText(g_GameSettings.m_szServerName, sizeof(g_GameSettings.m_szServerName) );

	// Let game module handle it
	Game_JoinMultiPlayerGame(sMenuItem);

	return bAcceptChoice;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill menuHostMulti.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t HostMultiInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

		if (rspGetResource(&g_resmgrShell, GUI_HOST_NAME_FILE, &ms_peditHostName) == 0)
			{
			// Set the text from the INI setting.  Note that we are changing a resource!
			ms_peditHostName->m_sMaxText = Net::MaxHostNameSize - 1;
			ms_peditHostName->SetText("%s", g_GameSettings.m_szHostName);
			ms_peditHostName->Compose();

			// Let menu know about it.
			pmenuCur->ami[0].pgui	= ms_peditHostName;
			}
		else
			{
			TRACE("HostMultiInit(): ms_presmgr->Get() failed.\n");
			sRes	= 1;
			}
		}
	else
		{
		// Release resources
		rspReleaseResource(&g_resmgrShell, &ms_peditHostName);

		// Clear menu's pointer.
		pmenuCur->ami[0].pgui	= NULL;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called by the menu API when an item is chosen in menuHostMulti.
//
////////////////////////////////////////////////////////////////////////////////
static bool HostMultiMenu(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	// If specified, get host name (BEFORE calling the game module!)
	if (ms_peditHostName != NULL)
		ms_peditHostName->GetText(g_GameSettings.m_szHostName, sizeof(g_GameSettings.m_szHostName) );

	// Let game module handle it
	Game_HostMultiPlayerGame(sMenuItem);

	return bAcceptChoice;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the Start Demo Game menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t StartDemoInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called by the menu API when an item is chosen in menuStartDemo.
//
////////////////////////////////////////////////////////////////////////////////
static bool StartDemoMenu(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	bool	bPlay	= false;

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	// Let game module handle it
	Game_StartDemoGame(sMenuItem);

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the options menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t OptionsInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
		return 0;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the options menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool OptionsChoice(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen or -1 for change of focus.
	{
		if (sMenuItem == -1)
			PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
		else
			PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);
		return true;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the game options menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t GameOptionsInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		int16_t sMenuItem = 0;

		if (rspGetResource(&g_resmgrShell, PLAYER_COLOR_GUI_FILE, &ms_ptxtColor) == 0)
			{
			// Keep in bounds just in case (anyone could type any number into the INI) . . .
			if (	g_GameSettings.m_sPlayerColorIndex >= CGameSettings::ms_sNumPlayerColorDescriptions
				||	g_GameSettings.m_sPlayerColorIndex >= CDude::MaxTextures
				|| g_GameSettings.m_sPlayerColorIndex < 0)
				{
				g_GameSettings.m_sPlayerColorIndex	= 0;
				}
			// Set the text from the INI setting. Note that we are changing a
			// resource!
			ms_ptxtColor->SetText("%s", CGameSettings::ms_apszPlayerColorDescriptions[g_GameSettings.m_sPlayerColorIndex]);
			ms_ptxtColor->Compose();
			pmenuCur->ami[sMenuItem++].pgui	= ms_ptxtColor;
			}
		else
			{
			TRACE("MultiOptionsInit(): rspGetResource() failed.\n");
			sRes	= 2;
			}
			
		RMultiBtn**	ppmb	= (RMultiBtn**)&(pmenuCur->ami[sMenuItem++].pgui);
		// Get check box for 'Crosshair'.
		if (rspGetResourceInstance(&g_resmgrShell, GUI_CHECKBOX_FILE, ppmb) == 0)
			{
			// Set the initial state.
			(*ppmb)->m_sState	= (g_GameSettings.m_sCrossHair != FALSE) ? 1 : 2;
			(*ppmb)->Compose();
			}
		else
			{
			TRACE("ControlsInit(): rspGetResource() failed.\n");
			sRes	= 1;
			}
		#ifdef KID_FRIENDLY_OPTION
		if (g_GameSettings.m_sCompletedAllLevelsMode == TRUE || g_GameSettings.m_sAprilFools == TRUE)
		{
			RMultiBtn**	kidMode	= (RMultiBtn**)&(pmenuCur->ami[sMenuItem++].pgui);
			if (rspGetResourceInstance(&g_resmgrShell, GUI_CHECKBOX_FILE, kidMode) == 0)
				{
				// Set the initial state.
				(*kidMode)->m_sState	= (g_GameSettings.m_sKidMode != FALSE) ? 1 : 2;
				(*kidMode)->Compose();
				}
			else
				{
				TRACE("ControlsInit(): rspGetResource() failed.\n");
				sRes	= 3;
				}
		} else {
			// Hide option.
			pmenuCur->ami[sMenuItem].sEnabled = FALSE;
			pmenuCur->ami[sMenuItem++].pszText = "\0";
		}
		#endif
		}
	else
		{
		int16_t sMenuItem = 0;

		if (ms_ptxtColor != NULL)
			{
			// Release resource.
			rspReleaseResource(&g_resmgrShell, &ms_ptxtColor);

			// Clear menu's pointer.
			TRACE("Before ms_ptxtColor assignment. sMenuItem = %d\n", sMenuItem);
			pmenuCur->ami[sMenuItem++].pgui	= NULL;
			TRACE("After ms_ptxtColor assignment. sMenuItem = %d\n", sMenuItem);
			}
			
		RMultiBtn**	ppmb	= (RMultiBtn**)&(pmenuCur->ami[sMenuItem++].pgui);
		if (*ppmb)
			{
			// Store new mouse usage setting.
			g_GameSettings.m_sCrossHair = ((*ppmb)->m_sState == 1) ? TRUE : FALSE;

			// Release resource.
			rspReleaseResourceInstance(&g_resmgrShell, ppmb);
			}

		#ifdef KID_FRIENDLY_OPTION
		if (g_GameSettings.m_sCompletedAllLevelsMode == TRUE || g_GameSettings.m_sAprilFools == TRUE)
		{
			RMultiBtn**	kidMode	= (RMultiBtn**)&(pmenuCur->ami[sMenuItem++].pgui);
			if (*kidMode)
				{
				// Store new kidmode setting.
				g_GameSettings.m_sKidMode = ((*kidMode)->m_sState == 1) ? TRUE : FALSE;
				
				// Release resource.
				rspReleaseResourceInstance(&g_resmgrShell, kidMode);
				}
				
			}
		#endif
		}
	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the game options menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool GameOptionsChoice(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	switch (sMenuItem)
		{
			case 0:
			// Increment and check to make sure we have a description and we have such a color . . .
			g_GameSettings.m_sPlayerColorIndex++;
			if (	g_GameSettings.m_sPlayerColorIndex >= CGameSettings::ms_sNumPlayerColorDescriptions
				||	g_GameSettings.m_sPlayerColorIndex >= CDude::MaxTextures)
				{
				g_GameSettings.m_sPlayerColorIndex	= 0;
				}

			// Set the text from the INI setting. Note that we are changing a
			// resource!
			ms_ptxtColor->SetText("%s", CGameSettings::ms_apszPlayerColorDescriptions[g_GameSettings.m_sPlayerColorIndex]);
			ms_ptxtColor->Compose();
			break;
			case 1:
			{
			// Toggle crosshair.
			g_GameSettings.m_sCrossHair = !g_GameSettings.m_sCrossHair;
			RMultiBtn*	pmb	= (RMultiBtn*)pmenuCurrent->ami[sMenuItem].pgui;
			ASSERT(pmb->m_type == RGuiItem::MultiBtn);
			pmb->NextState();
			pmb->Compose();
			break;
			}
#ifdef KID_FRIENDLY_OPTION
			case 2:
			// Toggle blood.
			g_GameSettings.m_sKidMode = !g_GameSettings.m_sKidMode;
			RMultiBtn*	kidMode	= (RMultiBtn*)pmenuCurrent->ami[sMenuItem].pgui;
			ASSERT(kidMode->m_type == RGuiItem::MultiBtn);
			kidMode->NextState();
			kidMode->Compose();
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
// Called to init or kill the play options menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t PlayOptionsInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

		RGuiItem*	pgui;

		// Get difficulty slider . . .
		if (rspGetResource(&g_resmgrShell, GUI_DIFFICULTY_FILE, &pgui) == 0)
			{
			// Get the scrollbar . . .
			ms_psbDifficulty	= (RScrollBar*)pgui->GetItemFromId(GUI_ID_DIFFICULTY_SLIDER);
			if (ms_psbDifficulty)
				{
				// Set the update call.
				ms_psbDifficulty->m_upcUser	= DifficultyScrollUpdate;
				// This is weird but it allows artie to finer tune the scroll thumb size.
				// We use the ID of the parent as the divisor for the scrollbar value.
				// Set the initial position.
				ms_psbDifficulty->SetPos(g_GameSettings.m_sDifficulty * pgui->m_lId);
				}

			// Let menu know about it.
			pmenuCur->ami[0].pgui	= pgui;
			}
		else
			{
			TRACE("PlayOptionsInit(): rspGetResource() failed.\n");
			sRes	= 1;
			}
		}
	else
		{
		if (pmenuCur->ami[0].pgui)
			{
			// Release the resource.
			rspReleaseResource(&g_resmgrShell, &pmenuCur->ami[0].pgui);
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the play options menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool PlayOptionsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,				// Current menu.
	int16_t	sMenuItem)					// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the video options menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t VideoOptionsInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,					// Current menu.
	int16_t	sInit)						// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

		if (rspGetResource(&g_resmgrShell, GUI_GAMMA_FILE, &ms_psbGamma) == 0)
			{
			// Set the update call.
			ms_psbGamma->m_upcUser	= GammaScrollUpdate;

			// Get range.
			int32_t	lMin, lMax;
			ms_psbGamma->GetRange(&lMin, &lMax);

			// Determine range of values.
			int32_t	lRange	= lMax - lMin;

			// Set the initial position.  Gamma value indicator will get set via callback.
			// Convert to gamma value by ratio.
			int32_t	lVal	= int32_t(float(lRange) / GAMMA_RANGE * (GetGammaLevel() - MIN_GAMMA_VAL) + 0.5) + lMin;
			ms_psbGamma->SetPos(lVal);

			// Let menu know about it.
			pmenuCur->ami[0].pgui	= ms_psbGamma;
			}
		else
			{
			TRACE("VideoOptionsInit(): rspGetResource() failed.\n");
			sRes	= 1;
			}
		}
	else
		{
		if (ms_psbGamma != NULL)
			{
			// Release resource.
			rspReleaseResource(&g_resmgrShell, &ms_psbGamma);
			}
		// Clear menu's pointer.
		pmenuCur->ami[0].pgui	= NULL;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the video options menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool VideoOptionsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,				// Current menu.
	int16_t	sMenuItem)					// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the audio options menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t AudioOptionsInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,					// Current menu.
	int16_t	sInit)						// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		if (rspGetResource(&g_resmgrShell, PLAYER_COLOR_GUI_FILE, &ms_ptxtLanguage) == 0)
			{
			// Keep in bounds just in case (anyone could type any number into the INI)
			if (g_GameSettings.m_sAudioLanguage >= NUM_LANGUAGES
				|| g_GameSettings.m_sAudioLanguage < 0)
				{
				#if LOCALE == JAPAN
					g_GameSettings.m_sAudioLanguage = JAPANESE_AUDIO;
				#else
					g_GameSettings.m_sAudioLanguage	= ENGLISH_AUDIO;
				#endif
				}
			// Set the text from the INI setting.
			ms_ptxtLanguage->SetText("%s", CGameSettings::ms_apszAudioLanguageDescriptions[g_GameSettings.m_sAudioLanguage]);
			ms_ptxtLanguage->Compose();
			pmenuCur->ami[2].pgui	= ms_ptxtLanguage;
			}
		else
			{
			TRACE("AudioOptionsInit(): rspGetResource() failed.\n");
			sRes	= 2;
			}
			
			if (rspGetResource(&g_resmgrShell, GUI_VOLUME_FILE, (RScrollBar**)&(pmenuCur->ami[3].pgui)) == 0)
				{
				RScrollBar* psb = (RScrollBar*)(pmenuCur->ami[3].pgui);
				int32_t lMin, lMax, lRange;
				psb->GetRange(&lMin, &lMax);
				lRange = lMax - lMin;
				psb->m_lButtonIncDec = lRange / 14; // using 16 here results in 18 stop points?!
				psb->m_lTrayIncDec = psb->m_lButtonIncDec;
				int16_t pos = g_GameSettings.m_sPainFrequency;
				// work around weird range issue halfway through slider
				if (pos < 10)
					pos--;
				psb->SetPos((pos * lRange / 16));
				psb->m_upcUser = PainFrequencyScrollUpdate;
				RGuiItem* pguiVal = psb->GetItemFromId(GUI_ID_VOLUME_VAL);
				if (pguiVal)
					{
					pguiVal->SetText("%s", "");
					pguiVal->Compose();
					}
				else
					{
					TRACE("AudioOptionsInit(): Could not get at pguiVal\n");
					}
				}
			else
				{
				TRACE("AudioOptionsInit(): Failed to initialise pain frequency slider.\n");
				sRes = 2;
				}
		}
	else
		{
			if (ms_ptxtLanguage != NULL)
				{
				// Release resource.
				rspReleaseResource(&g_resmgrShell, &ms_ptxtLanguage);

				// Clear menu's pointer.
				pmenuCur->ami[2].pgui	= NULL;
				}
			
			if (pmenuCur->ami[3].pgui)
				{
				rspReleaseResource(&g_resmgrShell, &(pmenuCur->ami[3].pgui));
				pmenuCur->ami[3].pgui = NULL;
				}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the audio options menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool AudioOptionsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,				// Current menu.
	int16_t	sMenuItem)					// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	if (sMenuItem == 2)
	{
		// Increment and check to make sure that we have such a language
			g_GameSettings.m_sAudioLanguage++;
			if (g_GameSettings.m_sAudioLanguage >= NUM_LANGUAGES
				||	g_GameSettings.m_sAudioLanguage < 0)
				{
				g_GameSettings.m_sAudioLanguage = 0;
				}

			// Set the text from the INI setting.
			ms_ptxtLanguage->SetText("%s", CGameSettings::ms_apszAudioLanguageDescriptions[g_GameSettings.m_sAudioLanguage]);
			ms_ptxtLanguage->Compose();
	}

	Game_AudioOptionsChoice(sMenuItem);
	
	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the volumes menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t VolumesInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

		int16_t i;
		for (i = 0; i < SampleMaster::MAX_NUM_SOUND_CATEGORIES && sRes == 0 && i < (NUM_ELEMENTS(pmenuCur->ami) - 2); i++)
			{
			if (rspGetResourceInstance(&g_resmgrShell, GUI_VOLUME_FILE, (RScrollBar**)&(pmenuCur->ami[i].pgui)) == 0)
				{
				RScrollBar*	psb	= (RScrollBar*)(pmenuCur->ami[i].pgui);
				ASSERT(psb->m_type == RGuiItem::ScrollBar);

				// Let the callback know which item this is.
				psb->m_ulUserData	= i;
				int32_t	lMin, lMax, lRange;
				psb->GetRange(&lMin, &lMax);
				lRange	= lMax - lMin;
				// Set the initial position.  
				psb->SetPos(g_GameSettings.m_asCategoryVolumes[i] * lRange / SampleMaster::UserMaxVolume);
				// Set the rate so based on the range.
				psb->m_lButtonIncDec	= lRange / SampleMaster::UserMaxVolume;
				psb->m_lTrayIncDec	= psb->m_lButtonIncDec;

				// Set the update call.  Note that we set the callback after setting the 
				// initial position so we don't get a callback for that.
				psb->m_upcUser		= VolumesScrollUpdate;

				// Note that we have to update the GUI val here b/c we don't allow the callback
				// until after we set the initial position of the scrollbar to avoid it playing
				// the example sample during that position change.

				// Get val indicator.
				RGuiItem*	pguiVal	= psb->GetItemFromId(GUI_ID_VOLUME_VAL);
				if (pguiVal)
					{
					pguiVal->SetText("%d", g_GameSettings.m_asCategoryVolumes[i]);
					pguiVal->Compose();
					}

				// Text.
				pmenuCur->ami[i].pszText	= SampleMaster::ms_apszSoundCategories[i];
				}
			else
				{
				TRACE("VolumesInit():  Failed to get resource.\n");
				sRes	= 1;
				}
			}

		// Make the second to last one defaults.
		pmenuCur->ami[i].sEnabled				= TRUE;
		pmenuCur->ami[i++].pszText				= g_pszRotationSetupMenu_RestoreDefaults;
		// Make the last one back.
#if 0
		static char szBack[]						= "";
		pmenuCur->ami[i].pszText				= szBack;
		pmenuCur->ami[i].sEnabled				= FALSE;
#endif
		pmenuCur->menuautoitems.sCancelItem	= i;
		}
	else
		{
		int16_t i;
		for (i = 0; i < SampleMaster::MAX_NUM_SOUND_CATEGORIES && sRes == 0 && pmenuCur->ami[i].pszText; i++)
			{
			// If this resource was allocated . . .
			if (pmenuCur->ami[i].pgui)
				{
				// Release resource.
				rspReleaseResourceInstance(&g_resmgrShell, &(pmenuCur->ami[i].pgui));
				}
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the volumes menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool VolumesChoice(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	switch (sMenuItem)
		{
		case -1:															// Selection change.
			break;
		case SampleMaster::MAX_NUM_SOUND_CATEGORIES + 1:	// Back.
			break;
		
		case SampleMaster::MAX_NUM_SOUND_CATEGORIES:			// Restore defaults.
			{
			int16_t i;
			for (i = 0; i < SampleMaster::MAX_NUM_SOUND_CATEGORIES && i < NUM_ELEMENTS(pmenuCurrent->ami); i++)
				{
				if (pmenuCurrent->ami[i].pgui)
					{
					int32_t	lMin, lMax, lRange;
					( (RScrollBar*)(pmenuCurrent->ami[i].pgui) )->GetRange(&lMin, &lMax);
					lRange	= lMax - lMin;

					((RScrollBar*)(pmenuCurrent->ami[i].pgui) )->SetPos(SampleMaster::UserDefaultVolume * lRange / SampleMaster::UserMaxVolume);

					// This'll keep the next sample from aborting the current so we can let the user
					// hear all the sounds when 'defaults' is chosen.
					ms_siLastSamplePlayed	= 0;
					}
				}
			break;
			}

		default:																// Play all.
			{
			// Play all samples simultaneously so user can hear the mixage.
			int16_t i;
			for (i = 0; i < SampleMaster::MAX_NUM_SOUND_CATEGORIES; i++)
				{
				// Play a sample in that category.
				PlaySample(									// Returns RSnd used to play the sample.
																// Does not fail.                       
					*(ms_apsmidVolumeTesters[i]),		// In:  Identifier of sample you want played.
					(SampleMaster::SoundCategory)i,	// In:  Sound Volume Category for user adjustment
					255,										// In:  Initial Sound Instance Volume (0 - 255)
					&ms_siLastSamplePlayed);			// Out: Handle for adjusting sound volume

				ms_siLastSamplePlayed	= 0;
				}
			break;
			}
		}

	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when the controls menu is initialized.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t ControlsInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

		RMultiBtn**	ppmb = NULL;

#if defined(ALLOW_JOYSTICK)
		int16_t	sMenuItem	= 4;

		ppmb = (RMultiBtn**)&(pmenuCur->ami[sMenuItem++].pgui);
		// Get check box for 'Use Joystick'.
		if (rspGetResourceInstance(&g_resmgrShell, GUI_CHECKBOX_FILE, ppmb) == 0)
			{
			// Set the initial state.
			(*ppmb)->m_sState	= (g_InputSettings.m_sUseJoy != FALSE) ? 1 : 2;
			(*ppmb)->Compose();
			}
		else
			{
			TRACE("ControlsInit(): rspGetResource() failed.\n");
			sRes	= 1;
			}
#else
		int16_t	sMenuItem	= 3;
#endif	// defined(ALLOW_JOYSTICK)

		// Get check box for 'Use Mouse'.
		if (rspGetResourceInstance(&g_resmgrShell, GUI_CHECKBOX_FILE, &ms_pmbCheckBox) == 0)
			{
			// Set the initial state.
			ms_pmbCheckBox->m_sState	= (g_InputSettings.m_sUseMouse != FALSE) ? 1 : 2;
			ms_pmbCheckBox->Compose();

			// Let menu know about it.
			pmenuCur->ami[sMenuItem++].pgui	= ms_pmbCheckBox;
			}
		else
			{
			TRACE("ControlsInit(): rspGetResource() failed.\n");
			sRes	= 1;
			}

		ppmb = (RMultiBtn**)&(pmenuCur->ami[sMenuItem++].pgui);

		//Menu Item 6. 'Use classic mouse' checkbox
		if (rspGetResourceInstance(&g_resmgrShell, GUI_CHECKBOX_FILE, ppmb) == 0) {

			//We cant use classic mouse when we dont use mouse at all
			(*ppmb)->m_sState = (g_InputSettings.m_sUseNewMouse == FALSE && g_InputSettings.m_sUseMouse != FALSE) ? 1 : 2;			
			(*ppmb)->Compose();

			//printf("On Create: m_sUseMouse: %i  m_sUseNewMouse %i\n", g_InputSettings.m_sUseMouse, g_InputSettings.m_sUseNewMouse);
		}
		else
		{
			TRACE("ControlsInit(): rspGetResource() failed.\n");
			sRes = 1;
		}


		// Get scrollbar for 'Mouse Sensitivity'.
		if (rspGetResourceInstance(&g_resmgrShell, GUI_MOUSE_SENSITIVITY_FILE, &ms_psbMouseSensitivityX) == 0)
			{
			// Set the initial state.

			// Set the callback.
			ms_psbMouseSensitivityX->m_upcUser			= MouseSensitivityScrollUpdate;
			// Set the value to change.
			ms_psbMouseSensitivityX->m_ulUserInstance	= (U64)&g_InputSettings.m_dMouseSensitivityX;


			// Set the initial position.  ms_psbGammaVal will get set via callback.
			ms_psbMouseSensitivityX->SetPos(g_InputSettings.m_dMouseSensitivityX * MOUSE_SENSITIVITY_DIVISOR);


			// Let menu know about it.
			pmenuCur->ami[sMenuItem++].pgui	= ms_psbMouseSensitivityX;
			}
		else
			{
			TRACE("ControlsInit(): rspGetResource() failed.\n");
			sRes	= 2;
			}

		// Get scrollbar for 'Mouse Sensitivity'.
		if (rspGetResourceInstance(&g_resmgrShell, GUI_MOUSE_SENSITIVITY_FILE, &ms_psbMouseSensitivityY) == 0)
			{
			// Set the initial state.

			// Set the callback.
			ms_psbMouseSensitivityY->m_upcUser			= MouseSensitivityScrollUpdate;
			// Set the value to change.
			ms_psbMouseSensitivityY->m_ulUserInstance	= (U64)&g_InputSettings.m_dMouseSensitivityY;


			// Set the initial position.  ms_psbGammaVal will get set via callback.
			ms_psbMouseSensitivityY->SetPos(g_InputSettings.m_dMouseSensitivityY * MOUSE_SENSITIVITY_DIVISOR);


			// Let menu know about it.
			pmenuCur->ami[sMenuItem++].pgui	= ms_psbMouseSensitivityY;
			}
		else
			{
			TRACE("ControlsInit(): rspGetResource() failed.\n");
			sRes	= 3;
			}
		}
	else
		{
		RMultiBtn**	ppmb;
#if defined(ALLOW_JOYSTICK)
		int16_t	sMenuItem	= 4;

		ppmb = (RMultiBtn**)&(pmenuCur->ami[sMenuItem++].pgui);
		if (*ppmb)
			{
			// Store new mouse usage setting.
			g_InputSettings.m_sUseJoy	= ((*ppmb)->m_sState == 1) ? TRUE : FALSE;

			// Release resource.
			rspReleaseResourceInstance(&g_resmgrShell, ppmb);
			}
#else
		int16_t	sMenuItem	= 3;
#endif	// defined(ALLOW_JOYSTICK)

		if (ms_pmbCheckBox)
			{
			// Store new mouse usage setting.
			g_InputSettings.m_sUseMouse	= (ms_pmbCheckBox->m_sState == 1) ? TRUE : FALSE;

			// Release resource.
			rspReleaseResourceInstance(&g_resmgrShell, &ms_pmbCheckBox);
			}

		// Clear menu's pointer.
		pmenuCur->ami[sMenuItem++].pgui	= NULL;

		//'Use New Mouse' code here
		ppmb = (RMultiBtn**)&(pmenuCur->ami[sMenuItem].pgui);
		if (*ppmb)
		{
			g_InputSettings.m_sUseNewMouse = (g_InputSettings.m_sUseMouse != FALSE && (*ppmb)->m_sState == 2) ? TRUE : FALSE;

			//printf("On exit: m_sUseMouse: %i  m_sUseNewMouse %i\n", g_InputSettings.m_sUseMouse, g_InputSettings.m_sUseNewMouse);
			// Release resource.
			rspReleaseResourceInstance(&g_resmgrShell, ppmb);
		}

		pmenuCur->ami[sMenuItem++].pgui = NULL;

		if (ms_psbMouseSensitivityX != NULL)
			{
			// Release resource.
			rspReleaseResourceInstance(&g_resmgrShell, &ms_psbMouseSensitivityX);
			}

		// Clear menu's pointer.
		pmenuCur->ami[sMenuItem++].pgui	= NULL;

		if (ms_psbMouseSensitivityY != NULL)
			{
			// Release resource.
			rspReleaseResourceInstance(&g_resmgrShell, &ms_psbMouseSensitivityY);
			}

		// Clear menu's pointer.
		pmenuCur->ami[sMenuItem++].pgui	= NULL;
		}


	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Call when a selection is changed or made on the controls menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool ControlsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	switch (sMenuItem)
		{
#if defined(ALLOW_JOYSTICK)
		case 4:
			{
			// Toggle 'Use Joystick'.
			RMultiBtn*	pmb	= (RMultiBtn*)pmenuCurrent->ami[sMenuItem].pgui;
			ASSERT(pmb->m_type == RGuiItem::MultiBtn);
			pmb->NextState();
			pmb->Compose();
			break;
			}
		case 5:
#else
		case 3:
#endif	// defined(ALLOW_JOYSTICK)

			// Toggle 'Use Mouse'.
			ms_pmbCheckBox->NextState();
			ms_pmbCheckBox->Compose();

			//If we disable Mouse also disable ClassicMouse
			if (ms_pmbCheckBox->m_sState == 2)
			{
				g_InputSettings.m_sUseNewMouse = TRUE;
				RMultiBtn*	pmb = (RMultiBtn*)pmenuCurrent->ami[6].pgui;
				pmb->m_sState = 2;
				pmb->Compose();
			}

			break;
		case 6:
			// Toggle 'Use Classic Mouse'
			RMultiBtn*	pmb = (RMultiBtn*)pmenuCurrent->ami[sMenuItem].pgui;
			ASSERT(pmb->m_type == RGuiItem::MultiBtn);
			pmb->NextState();
			pmb->Compose();

			//If we enable ClassicMouse also enable Mouse
			if (pmb->m_sState == 1)
			{
				g_InputSettings.m_sUseMouse = TRUE;
				RMultiBtn*	pmb = (RMultiBtn*)pmenuCurrent->ami[5].pgui;
				pmb->m_sState = 1;
				pmb->Compose();
			}

			break;
		}

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);
	
	// Call game.
	Game_ControlsMenu(sMenuItem);

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Macro for RotationInit().
//
////////////////////////////////////////////////////////////////////////////////
template <class ValType>
int16_t SetUpRotationScrollBar(		// Returns 0 on success.
	ValType*	pvtRotationVal,		// In:  Rotation value to tune via scrollbar.
	Menu*		pmenu,					// In:  Menu.
	int16_t		sChoiceIndex,			// In:  Index of choice.
	char*		pszResName)				// In:  Resource name.
	{
	int16_t	sRes	= 0;	// Assume success.

	RScrollBar*	psb	= NULL;

	if (rspGetResourceInstance(&g_resmgrShell, pszResName, &psb) == 0)
		{
		// Set the initial state.

		// Set the callback.
		switch (sizeof(ValType))
			{
			case 2:
				psb->m_upcUser				= RotationScrollUpdateShort;
				break;
			case 8:
				psb->m_upcUser				= RotationScrollUpdateDouble;
				break;
			default:
				TRACE("SetUpRotationScrollBar(): Unsupported value size.\n");
				break;
			}

		// Set the value to change.
		psb->m_ulUserInstance	= (U64)pvtRotationVal;


		// Set the initial position.  psb will get set via callback.
		psb->SetPos(*pvtRotationVal);
		}
	else
		{
		TRACE("SetUpRotationScrollBar(): rspGetResource() failed for \"%s\".\n", pszResName);
		sRes	= 1;
		}

	// Let menu know about it.
	pmenu->ami[sChoiceIndex].pgui	= psb;

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when the Rotation menu is initialized.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t RotationInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

		// Get scrollbars for rotation parameters.
		sRes	|= SetUpRotationScrollBar(&g_InputSettings.m_dMovingSlowDegreesPerSec,	pmenuCur, 0, GUI_ROTATION_FILE);
		sRes	|= SetUpRotationScrollBar(&g_InputSettings.m_dMovingFastDegreesPerSec,	pmenuCur, 1, GUI_ROTATION_FILE);
		sRes	|= SetUpRotationScrollBar(&g_InputSettings.m_dStillSlowDegreesPerSec,	pmenuCur, 2, GUI_ROTATION_FILE);
		sRes	|= SetUpRotationScrollBar(&g_InputSettings.m_dStillFastDegreesPerSec,	pmenuCur, 3, GUI_ROTATION_FILE);
		sRes	|= SetUpRotationScrollBar(&g_InputSettings.m_sTapRotationDegrees,			pmenuCur, 4, GUI_TAP_ROTATION_FILE);
		}
	else
		{
		int16_t	i;
		for (i = 0; pmenuCur->ami[i].pszText; i++)
			{
			if (pmenuCur->ami[i].pgui)
				{
				rspReleaseResourceInstance(&g_resmgrShell, (RScrollBar**)&(pmenuCur->ami[i].pgui) );
				}
			}
		}


	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Call when a selection is changed or made on the Rotation menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool RotationChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	switch (sMenuItem)
		{
		// Restore defaults.
		case 5:
			g_InputSettings.DefaultRotations();
			// Update sliders.
			( (RScrollBar*)(pmenuCurrent->ami[0].pgui) )->SetPos(g_InputSettings.m_dMovingSlowDegreesPerSec);
			( (RScrollBar*)(pmenuCurrent->ami[1].pgui) )->SetPos(g_InputSettings.m_dMovingFastDegreesPerSec);
			( (RScrollBar*)(pmenuCurrent->ami[2].pgui) )->SetPos(g_InputSettings.m_dStillSlowDegreesPerSec);
			( (RScrollBar*)(pmenuCurrent->ami[3].pgui) )->SetPos(g_InputSettings.m_dStillFastDegreesPerSec);
			( (RScrollBar*)(pmenuCurrent->ami[4].pgui) )->SetPos(g_InputSettings.m_sTapRotationDegrees);
			break;
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
// Called to init or kill the multiplyaer options menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t MultiOptionsInit(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,					// Current menu.
	int16_t	sInit)						// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{					   
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

		if (rspGetResource(&g_resmgrShell, PLAYER_NAME_GUI_FILE, &ms_peditName) == 0)
			{
			// Set the text from the INI setting.  Note that we are changing a resource!
			ms_peditName->m_sMaxText = Net::MaxPlayerNameSize - 1;
			ms_peditName->SetText("%s", g_GameSettings.m_szPlayerName);
			ms_peditName->Compose();

			// Let menu know about it.
			pmenuCur->ami[0].pgui	= ms_peditName;

			// If any errors occurred after getting resource, this function will be
			// called with sInit == FALSE.
			}
		else
			{
			TRACE("MultiOptionsInit(): rspGetResource() failed.\n");
			sRes	= 1;
			}

		if (rspGetResource(&g_resmgrShell, NET_PROTO_GUI_FILE, &ms_ptxtProto) == 0)
			{
			// Set the text from the INI setting.  Note that we are changing a 
			// resource!
			ms_ptxtProto->SetText("%s", RSocket::GetProtoName((RSocket::ProtoType)g_GameSettings.m_usProtocol));
			ms_ptxtProto->Compose();

			pmenuCur->ami[1].pgui   = ms_ptxtProto;
			}
		else
			{
			 TRACE("MultiOptionsIni(): rspGetResource() failed.\n");
			 sRes = 4;
			}

		if (rspGetResource(&g_resmgrShell, NET_CONNECTION_GUI_FILE, &ms_ptxtBandwidth) == 0)
			{
			// Set the text from the INI setting.  Note that we are changing a 
			// resource!
			if (g_GameSettings.m_sNetBandwidth >= Net::NumBandwidths)
				g_GameSettings.m_sNetBandwidth = Net::FirstBandwidth;

			ms_ptxtBandwidth->SetText("%s", Net::BandwidthText[g_GameSettings.m_sNetBandwidth]);
			ms_ptxtBandwidth->Compose();

			pmenuCur->ami[2].pgui   = ms_ptxtBandwidth;
			}
		else
			{
			TRACE("MultiOptionsInit(): rspGetResource() failed.\n");
			sRes	= 5;
			}
		}
	else
		{
		if (ms_peditName != NULL)
			{
			// Get the player name for storage purposes.
			ms_peditName->GetText(g_GameSettings.m_szPlayerName, sizeof(g_GameSettings.m_szPlayerName) );

			// Release resource.
			rspReleaseResource(&g_resmgrShell, &ms_peditName);

			// Clear menu's pointer.
			pmenuCur->ami[0].pgui	= NULL;
			}

		if (ms_ptxtProto)
			{
			// Release resource.
			rspReleaseResource(&g_resmgrShell, &ms_ptxtProto);

			// Clear menu's pointer.
			pmenuCur->ami[2].pgui	= NULL;
			}

		if (ms_ptxtBandwidth)
			{
			// Release resource.
			rspReleaseResource(&g_resmgrShell, &ms_ptxtBandwidth);

			// Clear menu's pointer.
			pmenuCur->ami[3].pgui	= NULL;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the multiplayer options menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool MultiOptionsChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCur,					// Current menu.
	int16_t	sMenuItem)					// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Toggle the protocol to the next one in the list
	switch (sMenuItem)
		{
		case 1:
			if (ms_ptxtProto != NULL)
				{
				g_GameSettings.m_usProtocol++;
				if (g_GameSettings.m_usProtocol >= RSocket::NumProtocols)
					g_GameSettings.m_usProtocol = RSocket::FirstProtocol;		
				ms_ptxtProto->SetText("%s", RSocket::GetProtoName((RSocket::ProtoType)g_GameSettings.m_usProtocol));
				ms_ptxtProto->Compose();
				}
			break;
		case 2:
			if (ms_ptxtBandwidth)
				{
				g_GameSettings.m_sNetBandwidth = (Net::Bandwidth)(g_GameSettings.m_sNetBandwidth + 1);
				if (g_GameSettings.m_sNetBandwidth >= Net::NumBandwidths)
					g_GameSettings.m_sNetBandwidth = Net::FirstBandwidth;

				// Set new text and realize.
				ms_ptxtBandwidth->SetText("%s", Net::BandwidthText[g_GameSettings.m_sNetBandwidth]);
				ms_ptxtBandwidth->Compose();
				}
			break;
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
// Called when a choice is made or changed on the Editor menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool EditorMenuChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	switch (sMenuItem)
		{
		case 0:
			Edit_Menu_Continue();
			break;
		case 1:
			break;
		case 2:
			Edit_Menu_ExitEditor();
			break;
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
// Called to init or kill the Mouse menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t MouseInit(			// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit == TRUE)
		{
		sRes	= InputSettingsDlg_InitMenu(pmenuCur);
		}
	else
		{
		sRes	= InputSettingsDlg_KillMenu(pmenuCur);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the Mouse menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool MouseChoice(		// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	InputSettingsDlg_Choice(pmenuCurrent, sMenuItem);

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the Key menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t KeyInit(			// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit == TRUE)
		{
		sRes	= InputSettingsDlg_InitMenu(pmenuCur);
		}
	else
		{
		sRes	= InputSettingsDlg_KillMenu(pmenuCur);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the Key menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool KeyChoice(			// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	InputSettingsDlg_Choice(pmenuCurrent, sMenuItem);

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the Joy menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t JoyInit(			// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit == TRUE)
		{
		sRes	= InputSettingsDlg_InitMenu(pmenuCur);
		}
	else
		{
		sRes	= InputSettingsDlg_KillMenu(pmenuCur);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the Joy menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool JoyChoice(			// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	InputSettingsDlg_Choice(pmenuCurrent, sMenuItem);

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the level select.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t LoadLevelInit(			// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.
	
	if (sInit == TRUE)
		{
		sRes	= Play_InitLevelSelectMenu(pmenuCur);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made on the level select.
//
////////////////////////////////////////////////////////////////////////////////
static bool LoadLevelChoice(			// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
	{
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);
		if (pmenuCurrent->ami[sMenuItem].sEnabled == TRUE)
		{
			TRACE("sMenuItem = %d\n", sMenuItem);
			Game_StartLevelOnce(sMenuItem);
		}
	}

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called to init or kill the Features menu.
//
////////////////////////////////////////////////////////////////////////////////
static int16_t FeaturesInit(		// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCur,				// Current menu.
	int16_t	sInit)					// TRUE, if initializing; FALSE, if killing.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sInit != FALSE)
		{
		RGuiItem::ms_print.SetFont(DEFAULT_GUI_FONT_HEIGHT, &g_fontPostal);

		int16_t	sItem;
		int16_t	sMaxFeatureItems	= NUM_ELEMENTS(pmenuCur->ami) - 1;
		for (sItem = 0; sItem < sMaxFeatureItems && pmenuCur->ami[sItem + 1].pszText && sRes == 0; sItem++)
			{
			RMultiBtn**	ppmb	= (RMultiBtn**)&(pmenuCur->ami[sItem].pgui);
			if (rspGetResourceInstance(&g_resmgrShell, GUI_CHECKBOX_FILE, ppmb) == 0)
				{
				// Set the initial state.
				int16_t	sState	= 1;
				switch (sItem)
					{
					case 0:
						sState	= (g_GameSettings.m_sAlphaBlend != FALSE) ? 1 : 2;
						break;
					case 1:
						sState	= (g_GameSettings.m_s3dFog != FALSE) ? 1 : 2;
						break;
					case 2:
						sState	= (g_GameSettings.m_sParticleEffects != FALSE) ? 1 : 2;
						break;
					case 3:
						sState	= (g_GameSettings.m_sVolumeDistance != FALSE) ? 1 : 2;
						break;
					case 4:
						sState	= (g_GameSettings.m_sPlayAmbientSounds != FALSE) ? 1 : 2;
						break;
					}
				
				(*ppmb)->m_sState	= sState;
				(*ppmb)->Compose();
				}
			else
				{
				TRACE("FeaturesInit(): rspGetResource() failed.\n");
				sRes	= 1;
				}
			}
		}
	else
		{
		int16_t	sMaxFeatureItems	= NUM_ELEMENTS(pmenuCur->ami) - 1;
		int16_t	sItem;
		for (sItem = 0; sItem < sMaxFeatureItems; sItem++)
			{
			RMultiBtn**	ppmb	= (RMultiBtn**)&(pmenuCur->ami[sItem].pgui);
			if (*ppmb)
				{
				// Get the final state.
				int16_t	sOn	= ( (*ppmb)->m_sState == 1) ? TRUE : FALSE;
				// Release the resource.
				rspReleaseResourceInstance(&g_resmgrShell, ppmb);
				// Update the feature.
				switch (sItem)
					{
					case 0:
						g_GameSettings.m_sAlphaBlend = sOn;
						break;
					case 1:
						g_GameSettings.m_s3dFog = sOn;
						break;
					case 2:
						g_GameSettings.m_sParticleEffects = sOn;
						break;
					case 3:
						g_GameSettings.m_sVolumeDistance = sOn;
						break;
					case 4:
						g_GameSettings.m_sPlayAmbientSounds	= sOn;
						break;
					}
				}
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Called when a choice is made or changed on the Features menu.
//
////////////////////////////////////////////////////////////////////////////////
static bool FeaturesChoice(	// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,			// Current menu.
	int16_t	sMenuItem)				// Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting.

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		{
		// If there's an associated GUI . . .
		if (pmenuCurrent->ami[sMenuItem].pgui)
			{
			// If it's a multistate btn . . .
			if (pmenuCurrent->ami[sMenuItem].pgui->m_type == RGuiItem::MultiBtn)
				{
				// Move to the next state and update visual components.
				RMultiBtn*	pmb	= (RMultiBtn*)pmenuCurrent->ami[sMenuItem].pgui;
				pmb->NextState();
				pmb->Compose();
				}
			}

		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);
		}

	return bAcceptChoice;
	}

////////////////////////////////////////////////////////////////////////////////
// GUI callbacks.

////////////////////////////////////////////////////////////////////////////////
//
// Callback from ms_psbGamma to update 'gamma' adjustment.
//
////////////////////////////////////////////////////////////////////////////////
static void GammaScrollUpdate(	// Returns nothing.
	RScrollBar*	psb)					// Scrollbar that got updated.
	{
	ASSERT(psb != NULL);

	// Get range.
	int32_t	lMin, lMax;
	psb->GetRange(&lMin, &lMax);

	// Determine range of values.
	int32_t	lRange	= lMax - lMin;

	// Set via scroll position.
	int32_t	lVal	= psb->GetPos();

	// Convert to gamma value by ratio.
	int16_t	sVal	= int16_t(GAMMA_RANGE / float(lRange) * (lVal - lMin) + 0.5) + MIN_GAMMA_VAL;

	SetGammaLevel(sVal);

	RGuiItem*	pguiGammaVal	= psb->GetItemFromId(GUI_ID_GAMMA_VAL);
	if (pguiGammaVal)
		{
		pguiGammaVal->SetText("%+02ld", lVal);
		pguiGammaVal->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from ms_psbDifficulty to update difficulty adjustment.
//
////////////////////////////////////////////////////////////////////////////////
static void DifficultyScrollUpdate(	// Returns nothing.
	RScrollBar*	psb)						// Scrollbar that got updated.
	{
	ASSERT(psb != NULL);

	// Get the parent.
	RGuiItem*	pguiParent	= psb->GetParent();
	// This is weird but it allows artie to finer tune the scroll thumb size.
	// We use the ID of the parent as the divisor for the scrollbar value.
	int16_t	sDivisor	= (int16_t)pguiParent->m_lId;

	g_GameSettings.m_sDifficulty	= (int16_t)psb->GetPos() / sDivisor;

	RGuiItem*	pguiVal	= psb->GetItemFromId(GUI_ID_DIFFICULTY_VAL);
	if (pguiVal)
		{
		pguiVal->SetText("%d", g_GameSettings.m_sDifficulty);
		pguiVal->Compose();
		}

	if (pguiParent)
		{
		RGuiItem*	pguiText	= psb->GetParent()->GetItemFromId(GUI_ID_DIFFICULTY_TEXT);
		if (pguiText)
			{
			// Add shadow.
			pguiText->m_sTextEffects			|= RGuiItem::Shadow;
			pguiText->m_u32TextShadowColor	= GetCurrentMenuBox()->m_u32TextShadowColor;

			pguiText->SetText("%s", ms_apszDifficultyDescriptions[g_GameSettings.m_sDifficulty - 1] );
			pguiText->Compose();
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback to update pain frequency adjustment.
//
////////////////////////////////////////////////////////////////////////////////
static void PainFrequencyScrollUpdate( // Returns nothing.
	RScrollBar* psb)					// Scrollbar that got updated.
	{
	ASSERT (psb != NULL);

	int32_t lMin, lMax;
	psb->GetRange(&lMin, &lMax);
	g_GameSettings.m_sPainFrequency = (int16_t)((((psb->GetPos() - lMin) * 16) / (lMax - lMin)));
	// work around weird range issue halfway through slider
	if (g_GameSettings.m_sPainFrequency < 10)
		g_GameSettings.m_sPainFrequency++;
	TRACE("g_GameSettings.m_sPainFrequency = %d\n", g_GameSettings.m_sPainFrequency);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from volume sliders to update corresponding volume adjustment.
//
////////////////////////////////////////////////////////////////////////////////
static void VolumesScrollUpdate(	// Returns nothing.
	RScrollBar*	psb)					// Scrollbar that got updated.
	{
	ASSERT(psb != NULL);

	SampleMaster::SoundCategory	sc		= (SampleMaster::SoundCategory)psb->m_ulUserData;
	int32_t									lMin, lMax, lRange;
	psb->GetRange(&lMin, &lMax);
	lRange	= lMax - lMin;

	// Get volume and scale to user volume.
	g_GameSettings.m_asCategoryVolumes[sc]	= (int16_t)(psb->GetPos() * SampleMaster::UserMaxVolume / lRange);

	SetCategoryVolume(sc, g_GameSettings.m_asCategoryVolumes[sc]);

	SampleMasterID*	psmid	= ms_apsmidVolumeTesters[sc];
	if (psmid == NULL)
		{
		psmid	= ms_apsmidVolumeTesters[0];
		}

	if (ms_siLastSamplePlayed)
		{
		// Abort last one.
		AbortSample(ms_siLastSamplePlayed);
		}

	// Play a sample in that category.
	PlaySample(								// Returns RSnd used to play the sample.
												// Does not fail.                       
		*psmid,								// In:  Identifier of sample you want played.
		sc,									// In:  Sound Volume Category for user adjustment
		255,									// In:  Initial Sound Instance Volume (0 - 255)
		&ms_siLastSamplePlayed);		// Out: Handle for adjusting sound volume

	// Get val indicator.
	RGuiItem*	pguiVal	= psb->GetItemFromId(GUI_ID_VOLUME_VAL);
	if (pguiVal)
		{
		pguiVal->SetText("%d", g_GameSettings.m_asCategoryVolumes[sc]);
		pguiVal->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from mouse sensitivity sliders to update corresponding 
// mouse sensitivity adjsutment.
//
////////////////////////////////////////////////////////////////////////////////
static void MouseSensitivityScrollUpdate(	// Returns nothing.
	RScrollBar* psb)								// Scrollbar that got updated.
	{
	// Update value.
	double*	pdSensitivity	= (double*)psb->m_ulUserInstance;
	*pdSensitivity = double(psb->GetPos() ) / MOUSE_SENSITIVITY_DIVISOR;

	RGuiItem*	pguiVal	= psb->GetItemFromId(GUI_ID_SENSITIVITY_VAL);
	if (pguiVal)
		{
		pguiVal->SetText("%f", *pdSensitivity);
		pguiVal->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from rotation scrollbars.
//
////////////////////////////////////////////////////////////////////////////////
static void RotationScrollUpdateDouble(	// Returns nothing.
	RScrollBar* psb)								// Scrollbar that got updated.
	{
	double*	pdRotVal	= (double*)psb->m_ulUserInstance;
	*pdRotVal			= double(psb->GetPos() );

	RGuiItem*	pguiVal	= psb->GetItemFromId(GUI_ID_ROTATION_VAL);
	if (pguiVal)
		{
		pguiVal->SetText("%d", (int16_t)(*pdRotVal) );
		pguiVal->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from rotation scrollbars.
//
////////////////////////////////////////////////////////////////////////////////
static void RotationScrollUpdateShort(	// Returns nothing.
	RScrollBar* psb)							// Scrollbar that got updated.
	{
	int16_t*	psRotVal	= (int16_t*)psb->m_ulUserInstance;
	*psRotVal			= int16_t(psb->GetPos() );

	RGuiItem*	pguiVal	= psb->GetItemFromId(GUI_ID_ROTATION_VAL);
	if (pguiVal)
		{
		pguiVal->SetText("%d", *psRotVal);
		pguiVal->Compose();
		}
	}


// Workaround the need for a platform-specific file selector, since this
//  requires external dependencies like GTK+ on Linux, and may not play well
//  with a fullscreen display mode on any platform. Better to just render
//  into the existing framebuffer.  --ryan.
#if 1 //PLATFORM_UNIX
static bool PickFileMenuChoice(Menu *pmenuCurrent, int16_t sMenuItem);

extern Menu	g_menuPickFile =
	{
	PICK_FILE_MENU_ID,

	// Position info.
		{	// x, y, w, h, sPosX, sPosY, sItemSpacingY, sIndicatorSpacingX,
		MENU_RECT_MD,					// menu x, y, w, h
		-120,								// menu header x offset
		MENU_HEAD_Y_MD,				// menu header y offset
		MENU_ITEM_X_MD,				// menu items x offset
		MENU_ITEM_Y_MD,				// menu items y offset
		MENU_ITEM_SPACE_Y_MD,		// vertical space between menu items
		MENU_ITEM_IND_SPACE_X_MD,	// horizontal space between indicator and menu items
		},

	// Background info.
		{	// pszFile, u32BackColor
		MENU_BG_MD, 
		MENU_BG_COLOR,		// Background color.
		PAL_SET_START,			// Starting palette index to set.
		PAL_SET_NUM,		// Number of entries to set.
		PAL_MAP_START,		// Starting index of palette entries that can be mapped to.
		PAL_MAP_NUM,		// Number of palette entries that can be mapped to.
		},

	// GUI settings.
		{	// sTransparent.
		TRUE,		// TRUE if GUI is to be BLiT with transparency.
		},

	// Flags.
		(MenuFlags)(MenuPosCenter | MenuBackTiled | MenuItemTextShadow | MenuHeaderTextShadow | MenuHeaderTextCenter),


	// Header and its font info.
		{	// pszHeaderText, pszFontFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor.
		NULL, // filled in by PickFile().
		SMASH_FONT,
		HEAD_FONT_HEIGHT,	// Height of font.
		HEAD_COLOR,			// Text RGBA.
		HEAD_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Font info.
		{	// pszFile, sHeight, u32ForeColor, u32BackColor, u32ShadowColor
		SMASH_FONT,
		ITEM_FONT_HEIGHT,	// Height of font.
		ITEM_COLOR,			// Text RGBA.
		ITEM_SHADOW_COLOR	// Text Shadow RGBA.
		},

	// Menu indicator.
		{	// pszFile, type
		MENU_INDICATOR,
		RImage::FSPR8,
		},

	// Menu callbacks.
		{	// fnInit, fnChoice,
		NULL,							// Called before menu is initialized.
		PickFileMenuChoice,	// Called when item is chosen.
		},

	// Menu auto items.
		{	// sDefaultItem, sCancelItem,
		0,		// Menu item (index in ami[]) selected initially.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		-1,	// Menu item (index in ami[]) chosen on cancel.
				// Negative indicates distance from number of items
				// (e.g., -1 is the last item).
		},
		
	// Menu items.
        // Filled in by PickFile().
		{	// pszText,						sEnabled,	pmenu,		pgui
			NULL							// Terminates list.
		},
	};

static volatile bool g_PickFileMenuDone = false;
static volatile const char *g_PickFileMenuChoice = NULL;
static bool PickFileMenuChoice(Menu *pmenuCurrent, int16_t sMenuItem)
{
    ASSERT(pmenuCurrent == &g_menuPickFile);

    if (sMenuItem == -1)  // focus change.
    {
        PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
        return true;
    }

	PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);
    g_PickFileMenuChoice = (const char *) pmenuCurrent->ami[sMenuItem].pszText;
    g_PickFileMenuDone = true;  // break loop in PickFile().
    return true;  // accept choice.
}

int16_t PickFile(const char *title, void (*enumer)(Menu *), char *buf, size_t bufsize)
{
    *buf = '\0';

    Menu *currentMenu = GetCurrentMenu();
    int Max = (sizeof(g_menuPickFile.ami) / sizeof(g_menuPickFile.ami[0])) - 1;

    g_menuPickFile.menuheader.pszHeaderText = (char *) title;
    memset(&g_menuPickFile.ami, '\0', sizeof (g_menuPickFile.ami));
    enumer(&g_menuPickFile);
    g_menuPickFile.menuautoitems.sCancelItem = Max;  // the null index
    g_PickFileMenuChoice = NULL;
    g_PickFileMenuDone = false;
    
	int16_t sResult = StartMenu(&g_menuPickFile, &g_resmgrShell, g_pimScreenBuf);
    if (sResult != 0)
    {
        TRACE("StartMenu failed! Can't pick file!\n");
        return -1;
    }

    while (!g_PickFileMenuDone)
    {
        if (rspGetQuitStatus())
            break;

		// Update the system, drive the audio, blah blah blah.
		UpdateSystem();

	    RInputEvent ie;
		ie.type	= RInputEvent::None;
		rspGetNextInputEvent(&ie);
		DoMenuInput(&ie, g_InputSettings.m_sUseJoy);
	    DoMenuOutput(g_pimScreenBuf);
    	rspUpdateDisplay();
    }

    if (g_PickFileMenuChoice != NULL)
    {
        strncpy(buf, (const char *) g_PickFileMenuChoice, bufsize);
        buf[bufsize-1] = '\0';  // just in case.
    }

    TRACE("PickFile: Going with %s.\n", buf);

    for (int i = 0; (i < Max) && (g_menuPickFile.ami[i].pszText); i++)
        free(g_menuPickFile.ami[i].pszText);

    // put the old menu back.
	StartMenu(currentMenu, &g_resmgrShell, g_pimScreenBuf);
    return ((g_PickFileMenuChoice == NULL) ? -1 : 0);
}
#endif  // PLATFORM_UNIX


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
