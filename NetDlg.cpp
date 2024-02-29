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
// NetDlg.CPP
// Project: Nostril (aka Postal)
// 
// History:
//		04/08/97 JMI	Started.
//
//		05/20/97	JMI	Now ms_pguiOK and ms_pguiCancel use SAFE_REF() so the
//							current GUI can, optionally, not have one or both GUI
//							items.
//
//		05/25/97	JMI	Integrated newest TCP/IP CNetServer/Client interface.
//							Still need to detect when the game is starting.
//							GUI could still use a little cleaning up.
//
//		05/26/97	JMI	Removed case for NetMsg::DuplicateName in reason
//							for JoinDeny message (it no longer exists).
//
//		05/26/97	JMI	Added sending and receiving of START_REALM.
//							Removed psNumPlayers and psMyPlayerNum parameters.
//
//		05/27/97	JMI	Added sending and receiving of LOAD_REALM before 
//							START_REALM.
//							Fixed bug in OnLoginMsg() which was telling the client
//							the wrong net ID.
//							Upgraded to utilize new functions RSocket::SetPort() and 
//							CNet::OpenPeerData().
//
//		05/27/97	JMI	OnJoinedMsg() now only adds the players' names to the
//							listbox of players.
//
//		05/28/97	JMI	Integrated new dialog.
//
//		05/28/97	JMI	Changed so the LOAD_REALM message motivates the leaving
//							of the dialog interface (was START_REALM that did it
//							before).
//
//		06/11/97	JMI	Removed unnecessary code in OnLoadRealmMsg().
//							Now uses Play_GetNextLevel to get realm names.
//
//		06/11/97	JMI	Now centers client/server dialogs on screen.
//
//		06/13/97 MJR	Restructed lots of stuff to consolidate client and
//							server code into a single function.
//							Implimented new version of START_GAME message.
//							Added code to get/release resources for new deluxe
//							dialog box.
//
//		06/15/97	JMI	Now deactivates client items in listbox as they are
//							added.
//							Also, now, instead of simply squashing the dialog to
//							exclude server controls in client mode, we simply hide
//							the server controls and center the remaining controls
//							in the dialog without altering its size.
//
//		06/16/97	JMI	Now uses g_fontSmall for dialogs.
//
//					MJR	Added use of watchdog timer for network blocking callbacks.
//
//					JMI	Changed use of g_fontSmall to g_fontBig.
//
//		06/17/97	JMI	Now only allows the server to process a DLG_OK.
//
//		06/30/97 MJR	Switched over to new GuiItem.h-supplied macros.
//
//		07/14/97 BRH	Changed call to Play_GetRealmInfo to include the
//							new challenge mode parameter.
//
//		08/02/97	JMI	Added an icon to watchdog timer for network blocking
//							callbacks and made it generally callable.
//
//		08/04/97 BRH	Added protocol parameter to net calls.
//
//		08/11/97	JMI	Changed a little of the structure of SetUpDlg() to allow
//							for more than just two types of dialogs.
//							Also, changed UpdateDialog() to use an RProcessGui which
//							simplifies that down to nearly nothing.  #if 0'd out old
//							stuff for now.
//							Checking in so Mike can compile so we can check his stuff
//							in so I can check this out again.
//
//					MJR	Modified to use new browsing functions and moved all
//							the find-the-host code into a separate function.
//
//					JMI	Now adds the items from the browse list to the dialog
//							(and drops them too (theoretically) ).
//
//		08/12/97	JMI	Dropped hosts were not setting the repaginate flag in
//							UpdateListBox().  Fixed.
//							DoNetGameDialog() now loops nearly all logic while 
//							browsing and no errors.
//
//		08/13/97 JMI	Cleaned up some more stuff.
//					MJR	Fixed bugs that kept us in a browsing loop.
//
//		08/14/97	JMI	Added global difficulty parameter to Play_GetRealmInfo().
//							I imagine it's not actually used for multiplayer levels
//							until we have options to play cooperative.
//
//		08/19/97	JMI	Added some unfinished usage of the multiplayer options
//							updating.  Needs to be updated to the correct spot,
//							either the game settings or the message.  Probably the
//							game settings which then get copied to the message.
//
//		08/19/97	JMI	Now uses the g_GameSettings for the multiplayer options.
//							Tinkered with chat messages that I thought weren't 
//							working but then realized they were being ignored b/c
//							the UpdateDialog() function was being called by the
//							net blocking callback so hopefully I didn't ruin anything
//							in that process... .
//							Genericized SetupDlg() to use UploadLinks().
//
//		08/19/97	JMI	Added a flag to UpdateDialog() indicating that it should
//							not clear any GUIs.  This is useful for when 
//							UpdateDialog() is called via callback (so we don't loose
//							any of the pressages processed via the callback).
//
//		08/20/97	JMI	Up/DownloadLinkInteger() are now hardwired to assume
//							'signed' type for String links b/c bool had a warning
//							for the trickery I was using to determine if the type
//							was signed.
//
//		08/20/97	JMI	Now chat history is a listbox.
//							Clicking the 'Send Chat' button moves focus back to the
//							edit field for better feedback.
//							Limits chats to 20.
//							Now updates client's view of host's multiplay options.
//							Now the server can select a player (for disconnection)
//							from the players list.
//							Implemented host option to disconnect a player.
//							Newest player is EnsureVisible()d in player list now.
//							Newest chat is EnsureVisible()d in chat list now.
//							Now sets number of connected players on two GUIs so
//							there can be a 3D effect.
//							Sends and responds to SETUP_GAME message.
//							Sets options in START_GAME message from dialog settings
//							now.
//							Updates settings even if dialog aborted for consistency.
//
//		08/21/97	JMI	Temporarily hardwired 'Rejuvenate' to ON and disallowed
//							user modifications of the checkbox.
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem() and occurrences
//							of rspUpdateDisplay() to UpdateDisplay().
//
//		08/23/97 MJR	Lots of changes in how errors are handled and other such
//							stuff.
//
//		08/23/97	JMI	Now list box items appear with a barely perceptible 
//							shadow that makes them alot easier to read.
//							Player colors now displayed as color descriptions.
//
//		08/25/97	JMI	Now the chat text edit field limits the text to the
//							message's capacity for text.
//							Also, got rid of some '***'s.
//
//		08/25/97	JMI	Now removes areas on chat items in chat box to make
//							for the border lines we hide to get more space.
//
//		08/27/97	JMI	Changed NetProbIcons functions to NetProbGui functions.
//							Also, instead of a DrawNetProbGui() there's a 
//							GetNetProbGui() so you can draw it, move it, change the
//							text, etc.
//
//		09/01/97 MJR	Nearing the end of a huge overhaul of networking.
//
//		09/02/97 MJR	It appears to work well after much testing, tuning, and
//							debugging.
//
//		09/03/97 MJR	Fixed a problem that made browsing for yourself fail
//							once in a while (the timeout timer was too short).
//
//		09/06/97	JMI	Now sets the Client's Game Options text field to use
//							the text shadow.  Also, selects the first item in the
//							level browser box in the Server's dialog.
//							Also, combined the status field and the chat box into
//							one 'Net Console'.
//
//		09/06/97	JMI	Now filters out the high word of ie.lKey when checking
//							for enter.  This filters out the the key flags (i.e., 
//							shift, control, alt, system).	 
//
//		09/07/97	JMI	Now BrowseForHosts() updates the display before 
//							displaying the browse dialog.
//
//		09/08/97 MJR	Fixed bug where net prob gui was showing up
//							immediately instead of after the watchdog expired.
//							Also cleaned up the erasing of the net prob gui.
//
//		09/09/97	JMI	Now initializes ms_pguiRetry to NULL in DlgBeGone().
//
//		09/09/97 MJR	Now detects protocol-not-supported and dispalys a
//							specific msg box describing the problem.
//
//		09/11/97 MJR	Now client checks to see whether the level that was
//							specified in the SETUP_GAME message is available, and
//							automatically sends a chat message that complains if it
//							isn't, in the hope that the host player will change it.
//
//							Now the host will send the realm name if only a single
//							realm is available, which tells everyone to only play
//							that realm.
//
//							Changed the dropped message, which was incorrectly using
//							the error message as the player's name.
//
//		09/11/97	JMI	Now only adds the SPECIFIC_MP_REALM_TEXT choice to the
//							host's level browser if ENABLE_PLAY_SPECIFIC_REALMS_ONLY
//							is defined. Otherwise, it chooses the first item in the 
//							listbox.
//
//		09/12/97 MJR	Reversed the #if for which method to use for filling
//							hood listbox.
//
//							Added hardwired realm stuff to SetupGame() when we're
//							in the specific realm mode.
//
//		09/18/97	JMI	There were some /"s instead of \"s.  If someone had
//							their syntax coloring on useful colors, they would've
//							seen this blaringly obvious color collage. :)
//
//		09/24/97	JMI	Fixed OnSetupGameMsg() to handle checking to see if the
//							local machine has the required realm using the title
//							that is passed in the SetupGame msg (was originally 
//							intended to use filenames but got hosed).
//
//		09/26/97	JMI	Now '-' is blocked at the UpdateDialog() level and never
//							reaches the GUIs so that the time limit and kill limit
//							edit fields cannot have a negative number enter in them
//							(of course you could enter the letter 'a' which looks
//							wrong too but I know one cares about that (you'd have to
//							be STUPID to do that but even smart people enter negative
//							numbers for time and kill limits) ).
//							Also, this keeps players from using '-' in their chat
//							strings.
//
//		11/20/97	JMI	Added cooperative flags and associated checkboxes.  Now
//							displays info on client side regarding cooperative mode
//							and carries all the necessary flagage around between
//							messages regarding cooperative mode as well.  Also, now
//							the level browse listbox can show either deathmatch or
//							cooperative levels (controlled by the user's cooperative 
//							checkbox setting).
//							Also, the user can choose operate checkbox deciding
//							whether to allow cooperative play mode (which is missiles
//							and bullets pass through fellow players).
//
//		11/25/97	JMI	Added platform conflict message and more informative
//							version conflict messages (also added version conflict 
//							message for server -- previous the error did not propagate
//							to this level).
//
//		11/25/97	JMI	Changed the server dialog .GUI to be loaded from the HD
//							instead of from the VD so we can guarantee the new asset
//							gets loaded (since they'll use their old Postal disc, we
//							cannot load the .GUI from the CD).
//
//////////////////////////////////////////////////////////////////////////////
//
// Deals with networking dialogs.  There is an interface for the server, 
//	client, and browser dialogs that deal heavily with user input, and GUI 
//	output.  The client/server messages are now handled by the black-box-like
// lower level.
//
//////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

#include "CompileOptions.h"
#include "GameSettings.h"
#include "game.h"
#include "play.h"
#include "update.h"
#include "NetDlg.h"
#include "netbrowse.h"


//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Common.
#define GUI_ID_OK						1001
#define GUI_ID_CANCEL				1002

// Common Client/Server dialog.
#define GUI_ID_PLAYERS_LISTBOX	1010
#define GUI_ID_CHAT_TEXT			1013
#define GUI_ID_CHAT_SEND			1015
#define GUI_ID_NET_CONSOLE			1016
#define GUI_ID_PLAYERS_STATIC1	7000
#define GUI_ID_PLAYERS_STATIC2	7001

// Client dialog.
#define GUI_ID_OPTIONS_STATIC		2000
#define GUI_ID_RETRY					9000

// Server dialog.
#define GUI_ID_DISCONNECT_PLAYER	6000
//#define GUI_ID_RESPAWN_PLAYERS	3000
#define GUI_ID_ENABLE_TIME_LIMIT	4000
#define GUI_ID_EDIT_TIME_LIMIT	4001
#define GUI_ID_ENABLE_KILL_LIMIT	5000
#define GUI_ID_EDIT_KILL_LIMIT	5001
#define GUI_ID_LEVEL_LISTBOX		1020
#define GUI_ID_SHOW_COOP_LEVELS	8000
#define GUI_ID_ENABLE_COOP_MODE	9000

// Browser dialog.
#define GUI_ID_HOST_LISTBOX		1010

#define SERVER_GUI					"res/shell/Server.gui"
#define CLIENT_GUI					"res/shell/Client.gui"
#define BROWSER_GUI					"res/shell/Browser.gui"
#define GUI_DIR						"menu/"

// Note that this is evaluated at compile time, so even
// if the ptr is NULL, it will work.
#define MAX_STATUS_STR				GUI_MAX_STR

// Maximum number of chat strings in the box.
#define MAX_CHAT_STRINGS			20

// Time between game setup net sends.
#define OPTIONS_SEND_FREQUENCY	2000

// Amount of time that must elapse after the first "Cancel" before an
// addition "Cancel" wil be recognized.
#define CANCEL_DELAY_TIME			1500

// If status hasn't been updated for this long, then we will display the default message
#define STATUS_MAX_DISPLAY_TIME	2000

// NOTE:  If we add anymore icons, we might as well just spend the extra few
// minutes making a separate file for them.
// Net problems icon file.
#define NET_PROB_GUI_FILE			"menu/netprob.gui"

// Initial location for net problems icon on screen.
#define NET_PROB_GUI_X				10
#define NET_PROB_GUI_Y				40

#define NET_PROB_TEXT_SHADOW_COLOR_INDEX	4

#define TERMINATING_GUI_ID			0x80000000

// Note that the following color indices are from the 'Artie Zone' of colors
// in the menu palette and, therefore, should not change.
#define LIST_ITEM_TEXT_SHADOW_COLOR_INDEX			220

// This has been remapped to a Win32 color to make it stand out more
#define NET_STATUS_MSG_COLOR_INDEX					247


//////////////////////////////////////////////////////////////////////////////
// THIS ENTIRE SECTION OF STRINGS SHOULD BE MOVED TO LOCALIZE.CPP!!!
//////////////////////////////////////////////////////////////////////////////

// Client-specific messages displayed in client dialog's status area
char* g_pszClientStat_NameTooLongForChat		= "Name too long, can't chat";
char* g_pszClientStat_YouWereDropped			= "Connection was lost";
char* g_pszClientStat_SomeoneDropped_s			= "\"%s\" is no longer connected";
char* g_pszClientStat_ServerAborted				= "Game aborted";
char* g_pszClientStat_ServerStarted				= "Starting game";
char* g_pszClientStat_Opened						= "Opened connection";
char* g_pszClientStat_Connected					= "Connected";
char* g_pszClientStat_JoinDenyTooMany			= "Can't join -- too many players";
char* g_pszClientStat_JoinDenyLowBandwidth	= "Can't join -- your bandwidth is too low";
char* g_pszClientStat_JoinDenyCantDropIn		= "Can't join -- game has already started";
char* g_pszClientStat_JoinDenyUnknown			= "Can't join -- don't know why";
char* g_pszClientStat_JoinAccepted				= "Joined";
char* g_pszClientStat_LoginAccepted_hd			= "Logged in (ID = %hd)";
char* g_pszClientStat_Startup						= "Trying to connect...";
char* g_pszClientStat_Default						= "Ready";
char* g_pszClientStat_Error_s						= "Network error (%s) -- aborting";
char* g_pszClientStat_Retrying					= "Retry...";

// Server-specific messages displayed in server dialog's status area
char* g_pszServerStat_InvalidDropReq_hd		= "Ignored invalid drop request (ID = %hd)";
char* g_pszServerStat_AcceptedClient			= "\"%s\" has joined";
char* g_pszServerStat_CantAcceptJoinReq		= "Can't grant join request (too many players)";
char* g_pszServerStat_InvalidChangeReq_hd		= "Ignored invalid change request (ID = %hd)";
char* g_pszServerStat_LoginAccepted_hd			= "Login accepted (id = %hd)";
char* g_pszServerStat_LoginDeniedVersion_ld	= "Login denied (obsolete client version %ld)";
char* g_pszServerStat_LoginDeniedMagic			= "Login denied (invalid signature)";
char* g_pszServerStat_Startup						= "Setting up connection...";
char* g_pszServerStat_Default						= "Ready";
char* g_pszServerStat_CantDropSelf				= "You can't drop yourself";
char* g_pszServerStat_PlayerErr					= "Player was dropped (bad connection)";

// General messages displayed in client or server dialog's status area
char* g_pszNetStat_Aborting						= "Aborting...";
char* g_pszNetStat_Starting						= "Starting game...";
char* g_pszNetStat_AttemptToDrop_s				= "Attempting to drop \"%s\"";
char* g_pszNetStat_UnhandledMsg					= "Ignoring extraneous message";

char* g_pszNetStat_NoError							= "No errors";
char* g_pszNetStat_ReceiveError					= "Network error (can't receive data)";
char* g_pszNetStat_InQFullError					= "Network error (input buffer is full)";
char* g_pszNetStat_OutQFullError					= "Network error (output buffer is full)";
char* g_pszNetStat_SendError						= "Network error (can't send data)";
char* g_pszNetStat_InQReadError					= "Network error (can't read data)";
char* g_pszNetStat_OutQWriteError				= "Network error (couldn't write data)";
char* g_pszNetStat_ConnectionError				= "Network error (bad connection)";
char* g_pszNetStat_TimeoutError					= "Network error (time-out)";
char* g_pszNetStat_ListenError					= "Network error (can't listen)";
char* g_pszNetStat_ConnectError					= "Network error (can't connect)";
char* g_pszNetStat_ConnectTimeoutError			= "Network error (connection attempt timed-out)";
char* g_pszNetStat_ClientVersionMismatchError_lu_lu	= "Version mismatch--dropping (Host ver is %lu -- Our ver is %lu)";
char* g_pszNetStat_ServerVersionMismatchError_lu_lu	= "Version mismatch--dropping client (Client ver is %lu -- Our ver is %lu)";
char* g_pszNetStat_CantOpenPeerSocketError	= "Network error (couldn't connect to other players)";
char* g_pszNetStat_LoginDeniedError				= "Login failed";
char* g_pszNetStat_JoinDeniedError				= "Host refused join request";
char* g_pszNetStat_UnknownError					= "Network error (general failure)";
char* g_pszNetStat_ProgramError					= "Network error (generic failure)";
#if defined(WIN32)
	char* g_pszNetStat_ClientPlatformMismatchError	= "Cannot login to host because it is a Mac";
	char* g_pszNetStat_ServerPlatformMismatchError	= "Cannot allow client to connect because it is a Mac";
#else
	char* g_pszNetStat_ClientPlatformMismatchError	= "Cannot login to host because it is a PC";
	char* g_pszNetStat_ServerPlatformMismatchError	= "Cannot allow client to connect because it is a PC";
#endif

// This is  what we say when the user has chosen a protocol that is not supported.
// There are two variations: one for if the user only has one choice (because we
// only support that one) and the other for if the user can try another choice.
char* g_pszNetOnlyProtocolUnsupported_s =
	"Your system does not support \"%s\", which is the required network protocol.\n"
	"\n"
	"This protocol must be added to your system before multiplayer mode can be used.";

char* g_pszNetProtocolUnsupported_s =
	"Your system does not support \"%s\", which is the currently selected network protocol.\n"
	"\n"
	"Either add this protocol to your system or choose a different protocol from the "
	"multiplayer options menu.";

// Text which is used to dynamically update one of the text fields on the client or server dialog
char* g_pszNetDlg_ConnectedPlayers_d			= "Connected Players: %d";

// Text which is used for the net problems GUI.
// WARNING: This is an EXTERN and is used by other modules!
char*	g_pszNetProb_General =
	"Network not responding.\nYou can wait or\npress " NET_PROB_GUI_ABORT_KEY_TEXT" to abort";

// Text to prefix net status messages, if any.
char* g_pszNetStatusMsgPrefix						= "> ";


//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

// Dialog actions
typedef enum
	{
	DLG_NOTHING,
	DLG_OK,
	DLG_CANCEL,
	DLG_CHAT,
	DLG_DISCONNECT_PLAYER,
	DLG_OPTIONS_UPDATED,
	DLG_RETRY
	} DLG_ACTION;

// Dialog types.
typedef enum
	{
	DLG_SERVER,
	DLG_CLIENT,
	DLG_BROWSER
	} DLG_TYPE;


// GUI/Var Link
typedef struct
	{
	typedef enum
		{
		Long,
		ULong,
		Short,
		UShort,
		Char,
		UChar,
		String,
		Bool,
		Gui
		} Type;

	long	lId;	// ID of GUI to link to.
	Type	type;
#if 1
	union
		{
		void*			pvLink;
		long*			pl;
		ULONG*		pul;
		short*		ps;
		USHORT*		pus;
		char*			pc;
		UCHAR*		puc;
		char*			psz;
		bool*			pb;
		RGuiItem**	ppgui;
		};
#else
	void*	pvLink;
#endif

	} GuiLink;

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

// Common.
static RGuiItem*	ms_pguiRoot			= NULL;		// Root of GUI tree for network interface.
static RGuiItem*	ms_pguiOk			= NULL;		// GUI that, once 'clicked', indicates acceptance.
static RGuiItem*	ms_pguiCancel		= NULL;		// GUI that, once 'clicked', indicates rejection.

// Client/Server common.
static RListBox*	ms_plbPlayers		= NULL;		// Listbox used by both types of net dialogs.
static RListBox*	ms_plbNetConsole	= NULL;		// List of net console (chat and status) strings.
static RGuiItem*	ms_pguiChatText	= NULL;		// Chat text.
static RGuiItem*	ms_pguiChatSend	= NULL;		// Chat send button.

// Server.
static RListBox*	ms_plbLevelBrowse	= NULL;		// Level browse listbox.
static RGuiItem*	ms_pguiDisconnectPlayer	= NULL;	// Disconnect a player button.
static RMultiBtn*	ms_pmbCoopLevels	= NULL;		// Coop levels checkbox.
static RMultiBtn*	ms_pmbCoopMode		= NULL;		// Coop mode checkbox.

// Client.
static RGuiItem*	ms_pguiOptions		= NULL;		// Options shown on client dialog.
static RGuiItem*	ms_pguiRetry		= NULL;		// Retry button on client dialog shown when needed.

// Browser.
static RListBox*	ms_plbHostBrowse	= NULL;		// Browse for host listbox.

// Other static vars.

static long			ms_lWatchdogTime = 0;			// Watchdog timer
static bool			ms_bNetBlockingAbort = false;	// Net blocking abort flag

static long			ms_lNumConsoleEntries		= 0;			// Track number of chat items.

static bool			ms_bGotSetupMsg = false;
static short		ms_sSetupRealmNum = 0;
static char			ms_szSetupRealmFile[Net::MaxRealmNameSize];
static long			ms_lSetupLastChatComplaint = 0;

static long			ms_lNextOptionsUpdateTime;		// Next time to send an options update.

static RTxt*		ms_ptxtNetProb	= NULL;			// Net problem GUI.
static bool			m_bNetWatchdogExpired	= false;	// Whether net blocking expired
static bool			ms_bCoopLevels	= false;			// true, to use cooperative levels.

// This is used by UpdateDialog() to perform GUI processing.
static RProcessGui	ms_pgDoGui;

// Linkable vars.
static bool			ms_bTimeLimit	= false;			// Enable Time Limit if true.
static bool			ms_bKillLimit	= false;			// Enable Kill Limit if true.
static bool			ms_bCoopMode = false;			// true, for cooperative mode (false for deathmatch).

static GuiLink			ms_aglServerLinkage[]	=
	{
		{ GUI_ID_DISCONNECT_PLAYER,	GuiLink::Gui,		&ms_pguiDisconnectPlayer,				},
//		{ GUI_ID_RESPAWN_PLAYERS,		GuiLink::Short,	&g_GameSettings.m_sHostRejuvenate,	},
		{ GUI_ID_ENABLE_TIME_LIMIT,	GuiLink::Bool,		&ms_bTimeLimit,							},
		{ GUI_ID_EDIT_TIME_LIMIT,		GuiLink::Short,	&g_GameSettings.m_sHostTimeLimit,	},
		{ GUI_ID_ENABLE_KILL_LIMIT,	GuiLink::Bool,		&ms_bKillLimit,							},
		{ GUI_ID_EDIT_KILL_LIMIT,		GuiLink::Short,	&g_GameSettings.m_sHostKillLimit,	},
		{ GUI_ID_LEVEL_LISTBOX,			GuiLink::Gui,		&ms_plbLevelBrowse,						},
		{ GUI_ID_SHOW_COOP_LEVELS,		GuiLink::Gui,		&ms_pmbCoopLevels,						},
		{ GUI_ID_ENABLE_COOP_MODE,		GuiLink::Gui,		&ms_pmbCoopMode,							},
		{ GUI_ID_ENABLE_COOP_MODE,		GuiLink::Bool,		&ms_bCoopMode,								},

		{ static_cast<long>(TERMINATING_GUI_ID), },	// Terminator.
	};

static GuiLink			ms_aglClientLinkage[]	=
	{
		{ GUI_ID_OPTIONS_STATIC,		GuiLink::Gui,		&ms_pguiOptions,							},   
		{ GUI_ID_RETRY,					GuiLink::Gui,		&ms_pguiRetry,								},
													 
		{ static_cast<long>(TERMINATING_GUI_ID), },	// Terminator.
	};

static GuiLink			ms_aglClientServerLinkage[]	=
	{
		{ GUI_ID_PLAYERS_LISTBOX,		GuiLink::Gui,		&ms_plbPlayers,							},   
		{ GUI_ID_CHAT_TEXT,				GuiLink::Gui,		&ms_pguiChatText,							},	 
		{ GUI_ID_CHAT_SEND,				GuiLink::Gui,		&ms_pguiChatSend,							},	   
		{ GUI_ID_NET_CONSOLE,			GuiLink::Gui,		&ms_plbNetConsole,						},	   
		{ GUI_ID_OK,						GuiLink::Gui,		&ms_pguiOk,									},
		{ GUI_ID_CANCEL,					GuiLink::Gui,		&ms_pguiCancel,							},
													 
		{ static_cast<long>(TERMINATING_GUI_ID), },	// Terminator.
	};

static GuiLink			ms_aglBrowserLinkage[]	=
	{
		{ GUI_ID_HOST_LISTBOX,			GuiLink::Gui,		&ms_plbHostBrowse,						},   
		{ GUI_ID_OK,						GuiLink::Gui,		&ms_pguiOk,									},
		{ GUI_ID_CANCEL,					GuiLink::Gui,		&ms_pguiCancel,							},
													 
		{ static_cast<long>(TERMINATING_GUI_ID), },	// Terminator.
	};


//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

static void AddConsoleMsg(	// Returns nothing.
	bool	bChat,				// In:  true for a chat message, or false for others (like net status).
	const char* pszFrmt,		// In:  sprintf style formatting.
	...);							// In:  Optional arguments based on context of pszFrmt.

// Get dialog resource
short DlgGetRes(											// Returns 0 if successfull, non-zero otherwise
	RGuiItem* pgui);										// I/O: Pointer to gui item

// Release dialog resource
void DlgReleaseRes(										// Returns 0 if successfull, non-zero otherwise
	RGuiItem* pgui);										// I/O: Pointer to gui item

// Net blocking callback
static short NetBlockingCallback(void);			// Returns 0 to continue normally, 1 to abort

static short BrowseForHost(
	CNetServer*	pserver,									// I/O: Server interface or NULL if none
	RSocket::Address* paddress);						// Out: Address returned here (if successfull)

static short FindSpecificSystem(
	RSocket::Address* paddress);						// Out: Address returned here (if successfull)

static short BrowseForSelf(
	CNetServer*	pserver,									// I/O: Server interface
	RSocket::Address* paddress);						// Out: Address returned here (if successfull)


//////////////////////////////////////////////////////////////////////////////
// Helper for UploadLinks() to handle all integer vars.
//////////////////////////////////////////////////////////////////////////////
template <class Int>				// Templated input type (can be unsigned).
void UploadLinkInteger(			// Returns nothing.
	RGuiItem*	pgui,				// In:  GUI to upload to.
	Int	i)							// In:  Input val to upload to GUI.
	{
	ASSERT(pgui);

	switch (pgui->m_type)
		{
		case RGuiItem::MultiBtn:
			{
			RMultiBtn*	pmb	= (RMultiBtn*)pgui;
			if (i)
				{
				pmb->m_sState	= 1;
				}
			else
				{
				pmb->m_sState	= 2;
				}
			break;
			}
		case RGuiItem::PushBtn:
			{
			RPushBtn*	ppushbtn	= (RPushBtn*)pgui;
			if (i)
				{
				ppushbtn->m_state	= RPushBtn::On;
				}
			else
				{
				ppushbtn->m_state	= RPushBtn::Off;
				}
			break;
			}
		default:
#if 0
			// Determine if signed (this is wierd but may work) . . .
			if ((Int)-1 < 0)
				{
				// Signed.
				pgui->SetText("%ld", (long)i);
				}
			else
				{
				// Unsigned.
				pgui->SetText("%lu", (ULONG)i);
				}
#else
			// Hardwire to signed b/c bool was displaying a warning regarding the
			// above comparison to determine the [un]signed nature of the templated
			// type.
			pgui->SetText("%ld", (long)i);
#endif
			break;
		}

	pgui->Compose();
	}

//////////////////////////////////////////////////////////////////////////////
// Helper for DownloadLinks() to handle all integer vars.
//////////////////////////////////////////////////////////////////////////////
template <class Int>				// Templated output type (can be unsigned).
void DownloadLinkInteger(		// Returns nothing.
	RGuiItem*	pgui,				// In:  GUI to download from.
	Int*	pi)						// Out: Input val to download into from GUI.
	{
	ASSERT(pgui);

	switch (pgui->m_type)
		{
		case RGuiItem::MultiBtn:
			{
			RMultiBtn*	pmb	= (RMultiBtn*)pgui;
			*pi = (pmb->m_sState == 1) ? 1 : 0;
			break;
			}
		case RGuiItem::PushBtn:
			{
			RPushBtn*	ppushbtn	= (RPushBtn*)pgui;
			*pi = (ppushbtn->m_state == RPushBtn::On) ? 1 : 0;
			break;
			}
		default:
#if 0
			// Determine if signed (this is wierd but may work) . . .
			if ((Int)-1 < 0)
				{
				// Signed.
				*pi = pgui->GetVal();
				}
			else
				{
				// Unsigned.
				*pi = strtoul(pgui->m_szText, NULL, 0);
				}
#else
			// Hardwire to signed b/c bool was displaying a warning regarding the
			// above comparison to determine the [un]signed nature of the templated
			// type.
			*pi = pgui->GetVal(); 
#endif
			break;
		}

	pgui->Compose();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Upload variables to their GUI links.
//
//////////////////////////////////////////////////////////////////////////////
static void UploadLinks(	// Returns nothing.
	GuiLink*		pagl,			// In:  Links to upload.
	RGuiItem*	pguiRoot)	// In:  GUI to upload to.
	{
	ASSERT(pguiRoot);

	while (pagl->lId != TERMINATING_GUI_ID)
		{
		// Get item.
		RGuiItem*	pgui	= pguiRoot->GetItemFromId(pagl->lId);
		if (pgui)
			{
			switch (pagl->type)
				{
				case GuiLink::Long:
					UploadLinkInteger(pgui, *pagl->pl);
					break;

				case GuiLink::ULong:
					UploadLinkInteger(pgui, *pagl->pul);
					break;
				
				case GuiLink::Short:
					UploadLinkInteger(pgui, *pagl->ps);
					break;
				
				case GuiLink::UShort:
					UploadLinkInteger(pgui, *pagl->pus);
					break;

				case GuiLink::Char:
					UploadLinkInteger(pgui, *pagl->pc);
					break;
				
				case GuiLink::UChar:
					UploadLinkInteger(pgui, *pagl->puc);
					break;

				case GuiLink::Bool:
					UploadLinkInteger(pgui, *pagl->pb);
					break;
				
				case GuiLink::String:
					pgui->SetText("%s", pagl->psz );
					break;

				// This may seem backward but it's more convenient.
				case GuiLink::Gui:
					*pagl->ppgui	= pgui;
					break;

				default:
					TRACE("UploadLinks(): Type %d not supported.\n",
						pagl->type);
					break;
				}
			}
		else
			{
			TRACE("UploadLinks(): GUI ID %d does not exist.\n", pagl->lId);
			}

		// Next.
		pagl++;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Download variables to their GUI links.
//
//////////////////////////////////////////////////////////////////////////////
static void DownloadLinks(	// Returns nothing.
	GuiLink*		pagl,			// In:  Links to upload.
	RGuiItem*	pguiRoot)	// In:  GUI to upload to.
	{
	ASSERT(pguiRoot);

	while (pagl->lId != TERMINATING_GUI_ID)
		{
		// Get item.
		RGuiItem*	pgui	= pguiRoot->GetItemFromId(pagl->lId);
		if (pgui)
			{
			switch (pagl->type)
				{
				case GuiLink::Long:
					DownloadLinkInteger(pgui, pagl->pl);
					break;
				
				case GuiLink::ULong:
					DownloadLinkInteger(pgui, pagl->pul);
					break;
				
				case GuiLink::Short:
					DownloadLinkInteger(pgui, pagl->ps);
					break;
				
				case GuiLink::UShort:
					DownloadLinkInteger(pgui, pagl->pus);
					break;
				
				case GuiLink::Char:
					DownloadLinkInteger(pgui, pagl->pc);
					break;
				
				case GuiLink::UChar:
					DownloadLinkInteger(pgui, pagl->puc);
					break;
				
				case GuiLink::Bool:
					DownloadLinkInteger(pgui, pagl->pb);
					break;
				
				case GuiLink::String:
					strcpy( pagl->psz, pgui->m_szText );
					break;
				
				case GuiLink::Gui:
					break;

				default:
					TRACE("DownloadLinks(): Type %d not yet supported.\n",
						pagl->type);
					break;
				}
			}
		else
			{
			TRACE("DownloadLinks(): GUI ID %d does not exist.\n", pagl->lId);
			}

		// Next.
		pagl++;
		}
	}

//////////////////////////////////////////////////////////////////////////////
// 
// Macro to activate certain parms to make our list item GUI's look nice and
// readable.
//
//////////////////////////////////////////////////////////////////////////////
inline void MakeMoreReadable(		// Returns nothing.
	RGuiItem*	pgui)					// In:  GUI to shadowify using global values.
	{
	ASSERT(pgui);

	if (pgui)
		{
		pgui->m_sTextEffects			= RGuiItem::Shadow;
		pgui->m_u32TextShadowColor	= LIST_ITEM_TEXT_SHADOW_COLOR_INDEX;
		pgui->m_sShowFocus			= FALSE;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// [Re]compose Options GUI with word wrap preserving original word wrap status.
//
//////////////////////////////////////////////////////////////////////////////
inline void ComposeOptions(void)
	{
	// Most likely someone has already checked this before calling this but
	// it doesn't hurt to be safe.
	if (ms_pguiOptions)
		{
		// Store old word wrap status so we can restore it when done.
		short sWordWrapWas	= (ms_pguiOptions->m_pprint->m_eModes & RPrint::WORD_WRAP) ? TRUE : FALSE;

		// Enable word wrap (not accessible from GUI editor currently).
		ms_pguiOptions->m_pprint->SetWordWrap(TRUE);

		ms_pguiOptions->Compose();

		// Reset word wrap.
		ms_pguiOptions->m_pprint->SetWordWrap(sWordWrapWas);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Clean/Reset Client dialog to defaults.
//
//////////////////////////////////////////////////////////////////////////////
static void CleanClientDlg(
	CNetClient* pnet,
	bool bRetryButton)
	{
	if (ms_pguiOptions)
		{
		// Set no-connection text for options GUI.
		ms_pguiOptions->SetText("Not currently connected to a host.");
		// Repaginate now!
		ComposeOptions();
		}

	if (ms_plbPlayers)
		{
		// Empty players listbox.
		ms_plbPlayers->RemoveAll();
		}

	int iNumPlayers = (pnet) ? pnet->GetNumPlayers() : 0;
		
	// This cheese updates the number of players displayed.  I cannot remember why we did not simply
	// make this one text shadowed GUI.
	RGuiItem*	pguiConnected	= ms_pguiRoot->GetItemFromId(GUI_ID_PLAYERS_STATIC1);
	RSP_SAFE_GUI_REF_VOID(pguiConnected, SetText(g_pszNetDlg_ConnectedPlayers_d, iNumPlayers));
	RSP_SAFE_GUI_REF_VOID(pguiConnected, Compose());
	pguiConnected	= ms_pguiRoot->GetItemFromId(GUI_ID_PLAYERS_STATIC2);
	RSP_SAFE_GUI_REF_VOID(pguiConnected, SetText(g_pszNetDlg_ConnectedPlayers_d, iNumPlayers));
	RSP_SAFE_GUI_REF_VOID(pguiConnected, Compose());

	
	if (ms_pguiRetry)
		{
		// Show as specified by caller.
		ms_pguiRetry->SetVisible(bRetryButton ? TRUE : FALSE);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Show the selectable network levels in the hood to play listbox.
//
//////////////////////////////////////////////////////////////////////////////
static short ShowLevels(void)						// Returns 0 on success.
	{
	short	sRes	= 0;	// Assume success.

	if (ms_plbLevelBrowse != NULL)
		{
		// Must be listbox.
		ASSERT(ms_plbLevelBrowse->m_type == RGuiItem::ListBox);
		// Get rid of existing entries.
		ms_plbLevelBrowse->RemoveAll();

		#if !defined(ENABLE_PLAY_SPECIFIC_REALMS_ONLY)
			// Add all the available realms.
			char	szRealm[RSP_MAX_PATH+1];
			char	szTitle[512];
			short	i	= 0;
			while (sRes == 0)
				{
				// Get realm name from realm prefs file
				sRes = Play_GetRealmInfo(true, ms_bCoopLevels, false, false, i, g_GameSettings.m_sDifficulty, szRealm, sizeof(szRealm), szTitle, sizeof(szTitle));
				if (sRes == 1)
					{
					// That's it.
					sRes	= 0;
					break;
					}
				else if (sRes == 0)
					{
					// Add to listbox.
					RGuiItem*	pguiLevel = ms_plbLevelBrowse->AddString(szTitle);
					if (pguiLevel != NULL)
						{
						// Activate shadow parameters.
						MakeMoreReadable(pguiLevel);
						pguiLevel->Compose();

						pguiLevel->m_lId = i;
						}
					else
						{
						TRACE("ShowLevels(): Failed to add string to listbox.\n");
						sRes	= -1;
						}
					}
				i++;
				}

		#else

			// Add the one and only level option.
			RGuiItem*	pguiLevel = ms_plbLevelBrowse->AddString(SPECIFIC_MP_REALM_TEXT);
			if (pguiLevel != NULL)
				{
				// Activate shadow parameters.
				MakeMoreReadable(pguiLevel);
				pguiLevel->Compose();

				pguiLevel->m_lId = SPECIFIC_MP_REALM_NUM;
				}
			else
				{
				TRACE("ShowLevels(): Failed to add string to listbox.\n");
				sRes	= -1;
				}

		#endif	// ENABLE_PLAY_SPECIFIC_REALMS_ONLY

		// Select the first item in the listbox.
		ms_plbLevelBrowse->SetSel(ms_plbLevelBrowse->GetFirst() );
		// Update listbox.
		ms_plbLevelBrowse->AdjustContents();
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Be done with the current dialog and related settings.
//
//////////////////////////////////////////////////////////////////////////////
static void DlgBeGone(void)
	{
	// Clean up processor.
	ms_pgDoGui.Unprepare();

	if (ms_pguiRoot != NULL)
		{
		delete ms_pguiRoot;
		}

	ms_pguiRoot					= NULL;
	ms_pguiOk					= NULL;
	ms_pguiCancel				= NULL;
	ms_plbPlayers				= NULL;
	ms_plbNetConsole			= NULL;
	ms_pguiChatText			= NULL;
	ms_pguiChatSend			= NULL;
	ms_plbLevelBrowse			= NULL;
	ms_pguiOptions				= NULL;
	ms_pguiDisconnectPlayer	= NULL;
	ms_pmbCoopLevels			= NULL;
	ms_pmbCoopMode				= NULL;
	ms_plbHostBrowse			= NULL;
	ms_pguiRetry				= NULL;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Setup the specified dialog and related settings.
//
//////////////////////////////////////////////////////////////////////////////
static short SetupDlg(		// Returns 0 on success.
	char*	pszGuiFile,			// In:  Full path to GUI file.
	DLG_TYPE	type)				// In:  Type of dialog.
	{
	short	sRes	= 0;	// Assume success.

	// Make sure everything is clean.
	DlgBeGone();

	// Anh...reset these here.
	ms_lNumConsoleEntries					= 0;
	ms_lNextOptionsUpdateTime	= 0;

	// Create a dialog.  We have to take this approach instea of the nicer
	// LoadInstantiate() method because we need to install callbacks for
	// getting and releasing resources, which we can only do if we already
	// have the gui item (the dialog in this case).  With LoadInstantiate(),
	// the dialog doesn't exist until after the call, and by then the resource
	// has already been loaded (or failed to load in this case because the
	// gui stuff can't possibly know the correct path for loading it).
	ms_pguiRoot = new RDlg;
	if (ms_pguiRoot != NULL)
		{
		// Setup callbacks for getting and releasing resources
		ms_pguiRoot->m_fnGetRes = DlgGetRes;
		ms_pguiRoot->m_fnReleaseRes = DlgReleaseRes;

		// Set font.  The size in this call matters not b/c each GUI item
		// has a size it uses when printing.
		ms_pguiRoot->m_pprint->SetFont(15, &g_fontPostal);

		// Load the gui description file
		sRes = ms_pguiRoot->Load(pszGuiFile);
		if (sRes == 0)
			{

			// Set focus to first child control or none if there's none.
			ms_pguiRoot->SetFocus(ms_pguiRoot->m_listguiChildren.GetHead());

			// The special case stuff.
			switch (type)
				{
				case DLG_CLIENT:
					{
					// Upload client stuff.
					UploadLinks(ms_aglClientLinkage, ms_pguiRoot);

					// Upload stuff common to client/server.
					UploadLinks(ms_aglClientServerLinkage, ms_pguiRoot);

					// If exists . . .
					if (ms_plbPlayers != NULL)
						{
						// Must be listbox.
						ASSERT(ms_plbPlayers->m_type == RGuiItem::ListBox);
						// Get rid of existing entries.
						ms_plbPlayers->RemoveAll();
						}

					// If exists . . .
					if (ms_pguiChatText != NULL)
						{
						// Must be edit.
						ASSERT(ms_pguiChatText->m_type == RGuiItem::Edit);
						// Limit text.
						( (REdit*)ms_pguiChatText)->m_sMaxText	= Net::MaxChatSize - 1;
						}

					// Make the options GUI more readable.
					if (ms_pguiOptions)
						{
						MakeMoreReadable(ms_pguiOptions);
						}

					// Set title text.
					ms_pguiRoot->SetText(
						"%s [%s]", 
						ms_pguiRoot->m_szText, 
						g_GameSettings.m_szPlayerName);
					ms_pguiRoot->Compose();

					// Start with a clean slate.
					CleanClientDlg(0, false);

					break;
					}
				case DLG_SERVER:
					{
					// Upload server stuff.
					UploadLinks(ms_aglServerLinkage, ms_pguiRoot);

					// Upload stuff common to client/server.
					UploadLinks(ms_aglClientServerLinkage, ms_pguiRoot);

					// If exists . . .
					if (ms_plbPlayers != NULL)
						{
						// Must be listbox.
						ASSERT(ms_plbPlayers->m_type == RGuiItem::ListBox);
						// Get rid of existing entries.
						ms_plbPlayers->RemoveAll();
						}

					// If exists . . .
					if (ms_pguiChatText != NULL)
						{
						// Must be edit.
						ASSERT(ms_pguiChatText->m_type == RGuiItem::Edit);
						// Limit text.
						( (REdit*)ms_pguiChatText)->m_sMaxText	= Net::MaxChatSize - 1;
						}

					// If exists . . .
					if (ms_pmbCoopLevels)
						{
						// Must be multibtn.
						ASSERT(ms_pmbCoopLevels->m_type == RGuiItem::MultiBtn);
						// Set state to last choice.
						ms_pmbCoopLevels->m_sState	= (ms_bCoopLevels == false) ? 1 : 2;
						// Recompose with new state.
						ms_pmbCoopLevels->Compose();
						}

					// If exists . . .
					if (ms_pmbCoopMode)
						{
						// Must be multibtn.
						ASSERT(ms_pmbCoopMode->m_type == RGuiItem::MultiBtn);
						// Set state to last choice.
						ms_pmbCoopMode->m_sState	= (ms_bCoopMode == false) ? 2 : 1;
						// Recompose with new state.
						ms_pmbCoopMode->Compose();
						}

					// Fill level browser listbox.
					ShowLevels();

					// Set title text.
					ms_pguiRoot->SetText(
						"%s [%s]", 
						ms_pguiRoot->m_szText, 
						g_GameSettings.m_szPlayerName);
					ms_pguiRoot->Compose();

					break;
					}
				case DLG_BROWSER:
					{
					// Upload browser linkage.
					UploadLinks(ms_aglBrowserLinkage, ms_pguiRoot);

					// If exists . . .
					if (ms_plbHostBrowse)
						{
						// Must be listbox.
						ASSERT(ms_plbHostBrowse->m_type == RGuiItem::ListBox);
						// Get rid of existing entries.
						ms_plbHostBrowse->RemoveAll();
						}
					break;
					}
				}

			// Center.
			ms_pguiRoot->Move(
				g_pimScreenBuf->m_sWidth / 2 - ms_pguiRoot->m_im.m_sWidth / 2,
				g_pimScreenBuf->m_sHeight / 2 - ms_pguiRoot->m_im.m_sHeight / 2);

			// Make visible.
			ms_pguiRoot->SetVisible(TRUE);

			// Prepare GUI processor.
			ms_pgDoGui.m_sFlags	= RProcessGui::NoCleanScreen;

			ms_pgDoGui.Prepare(ms_pguiRoot, ms_pguiOk, ms_pguiCancel);
			}
		else
			{
			TRACE("SetupDlg(): Failed to load .gui file!\n");
			}
		}
	else
		{
		TRACE("SetupDlg(): Failed to allocate RDlg!\n");
		sRes = -1;
		}

	// If any errors . . .
	if (sRes != 0)
		{
		DlgBeGone();
		}

	return sRes;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Get dialog resource
//
//////////////////////////////////////////////////////////////////////////////
short DlgGetRes(											// Returns 0 if successfull, non-zero otherwise
	RGuiItem* pgui)										// I/O: Pointer to gui item
	{
	short sResult = 0;

	// Release resources first (just in case)
	DlgReleaseRes(pgui);

	// Allocate and load new resources.  We get the name of the file (which
	// is ASSUMED to have NO PATH!!) from the gui itself, then tack on the
	// path we need and get the resource from the resource manager.
	char szFile[RSP_MAX_PATH * 2];
	sprintf(szFile, "%s%s", GUI_DIR, pgui->m_szBkdResName);

	if (rspGetResource(&g_resmgrShell, szFile, &pgui->m_pimBkdRes) == 0)
		{
		// Set palette
		ASSERT(pgui->m_pimBkdRes->m_pPalette != NULL);
		ASSERT(pgui->m_pimBkdRes->m_pPalette->m_type == RPal::PDIB);
		rspSetPaletteEntries(
			0,
			230,
			pgui->m_pimBkdRes->m_pPalette->Red(0),
			pgui->m_pimBkdRes->m_pPalette->Green(0),
			pgui->m_pimBkdRes->m_pPalette->Blue(0),
			pgui->m_pimBkdRes->m_pPalette->m_sPalEntrySize);

		// Update hardware palette.
		rspUpdatePalette();
		}
	else
		{
		sResult = -1;
		TRACE("DlgGetRes(): Failed to open file '%s'\n", FullPathVD(szFile));
		}

	return sResult;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Release dialog resource
//
//////////////////////////////////////////////////////////////////////////////
void DlgReleaseRes(										// Returns 0 if successfull, non-zero otherwise
	RGuiItem* pgui)										// I/O: Pointer to gui item
	{
	rspReleaseResource(&g_resmgrShell, &pgui->m_pimBkdRes);
	}


//////////////////////////////////////////////////////////////////////////////
//
// Process GUI tree through one iteration.
//
//////////////////////////////////////////////////////////////////////////////
static DLG_ACTION UpdateDialog(						// Returns dialog action
	RGuiItem*	pguiRoot,								// In:  GUI to process via user input.
	bool			bReset)									// In:  If false, does not reset any
																// GUIs before returning their action.
																// Note that this means that the next
																// time through, we'll probably return
																// the same action.
	{
	// Get next input event.
	RInputEvent	ie;
	// Make sure we start with no event.
	ie.type	= RInputEvent::None;
	rspGetNextInputEvent(&ie);

	// Block minus sign right here (note that this will keep players 
	// from being able to use '-' in chat strings).
	if (ie.type == RInputEvent::Key)
		{
		switch (ie.lKey & 0x0000FFFF)
			{
			case '-':
				ie.type	= RInputEvent::None;
				break;
			}
		}

	// When we lose the focus, this might need to be updated.
	// Should we do it everytime?
	rspNameBuffers(&g_pimScreenBuf);

	// Process GUI through an iteration.
	long	lPressedId	= ms_pgDoGui.DoModeless(pguiRoot, &ie, g_pimScreenBuf);

	// If OK chosen or enter pressed . . .
	if (lPressedId == GUI_ID_OK || (ie.type == RInputEvent::Key && (ie.lKey & 0x0000FFFF) == '\r') )
		{
		// If there's a focus . . .
		if (RGuiItem::ms_pguiFocus)
			{
			// If in chatbox . . .
			if (RGuiItem::ms_pguiFocus == ms_pguiChatText)
				{
				// Send chat.
				RSP_SAFE_GUI_REF_VOID(ms_pguiChatSend, SetClicked(TRUE) );
				}
			else if (RGuiItem::ms_pguiFocus == ms_pguiRetry)
				{
				RSP_SAFE_GUI_REF_VOID(ms_pguiRetry, SetClicked(TRUE) );
				}
			else
				{
				// OK.
				RSP_SAFE_GUI_REF_VOID(ms_pguiOk, SetClicked(TRUE) );
				}
			}
		else
			{
			// OK.
			RSP_SAFE_GUI_REF_VOID(ms_pguiOk, SetClicked(TRUE) );
			}
		}

	// Return proper action based on what the user clicked on
	DLG_ACTION action = DLG_NOTHING;
	if (RSP_SAFE_GUI_REF(ms_pguiOk, IsClicked()))
		{
		if (bReset)
			{
			RSP_SAFE_GUI_REF_VOID(ms_pguiOk, SetClicked(FALSE));
			}
		
		action = DLG_OK;
		}

	if (RSP_SAFE_GUI_REF(ms_pguiCancel, IsClicked()))
		{
		if (bReset)
			{
			RSP_SAFE_GUI_REF_VOID(ms_pguiCancel, SetClicked(FALSE));
			}

		action = DLG_CANCEL;
		}

	if (RSP_SAFE_GUI_REF(ms_pguiRetry, IsClicked() ) )
		{
		if (bReset)
			{
			RSP_SAFE_GUI_REF_VOID(ms_pguiRetry, SetClicked(FALSE) );
			}

		action	= DLG_RETRY;
		}

	if (RSP_SAFE_GUI_REF(ms_pguiChatSend, IsClicked()))
		{
		if (bReset)
			{
			if (ms_pguiChatSend)
				{
				ms_pguiChatSend->SetClicked(FALSE);

				// If focus is on the chat send . . .
				if (RGuiItem::ms_pguiFocus == ms_pguiChatSend && ms_pguiChatText)
					{
					// Switch back to the edit...makes sense on the user interface, I think.
					RGuiItem::SetFocus(ms_pguiChatText);
					}
				}
			}

		action = DLG_CHAT;
		}

	if (RSP_SAFE_GUI_REF(ms_pguiDisconnectPlayer, IsClicked() ) )
		{
		if (bReset)
			{
			RSP_SAFE_GUI_REF_VOID(ms_pguiDisconnectPlayer, SetClicked(FALSE) );
			}

		action	= DLG_DISCONNECT_PLAYER;
		}

	// If no other action . . .
	if (action == DLG_NOTHING)
		{
		// Temporarily timed based. ***
		long	lCurTime	= rspGetMilliseconds();
		if (lCurTime > ms_lNextOptionsUpdateTime)
			{
			if (bReset)
				{
				ms_lNextOptionsUpdateTime	= lCurTime + OPTIONS_SEND_FREQUENCY;
				}

			action	= DLG_OPTIONS_UPDATED;
			}
		}

	if (ms_pmbCoopLevels)
		{
		// If not pressed . . .
		if (ms_pmbCoopLevels->m_sState > 0)
			{
			bool	bCoopLevels	= (ms_pmbCoopLevels->m_sState == 2) ? true : false;

			// If status has changed . . .
			if (bCoopLevels != ms_bCoopLevels)
				{
				// Set new value.
				ms_bCoopLevels	= bCoopLevels;

				// Repaginate.
				ShowLevels();

				// If showing deathmatch levels . . .
				if (bCoopLevels == false && ms_pmbCoopMode)
					{
					// Since cooperative mode in deathmatch levels makes little
					// sense, let's automagically switch to deathmatch mode for
					// convenience sake.
					ms_pmbCoopMode->m_sState	= 2;
					ms_pmbCoopMode->Compose();
					}
				}
			}
		}

	return action;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Update hosts listbox for browser.
//
//////////////////////////////////////////////////////////////////////////////
static short UpdateListBox(					// Returns 0 on success.
	RListBox*	plb,								// In:  Browser listbox.
	CNetBrowse::Hosts* phostslistPersist,	// In:  Hosts.
	CNetBrowse::Hosts* phostslistAdded,		// In:  Hosts to add to listbox.
	CNetBrowse::Hosts* phostslistDropped)	// In:  Hosts to drop from listbox.
	{
	short	sResult	= 0;	// Assume success.

	if (plb)
		{
		CNetBrowse::Hosts::Pointer	i;
		bool	bRepaginate	= false;
		
		// Look for unGUIed entries . . .
		for (i = phostslistPersist->GetHead(); i; i = phostslistPersist->GetNext(i))
			{
			CNetBrowse::CHost* phost = &(phostslistPersist->GetData(i) );
			// If not yet GUIed . . .
			if ( !(phost->m_u32User) )
				{
				RGuiItem*	pgui	= plb->AddString(phost->m_acName);
				if (pgui)
					{
					// Activate shadow parameters.
					MakeMoreReadable(pgui);
					pgui->Compose();
					// Point GUI at entry.
					pgui->m_ulUserData	= (ULONG)phost;
					// Successfully added entry.
					phost->m_u32User	= (U32)pgui;
					// Note that we updated the dialog and will need to re-adjust
					// fields and recompose.
					bRepaginate	= true;
					}
				else
					{
					TRACE("UpdateListBox():  Failed to add a host to the listbox.\n");
//					sResult	= -1;	// Error?
					}
				}
			}

		// For whatever it's worth, let's do dropped first so it'll be a little
		// faster.  On second thought, let's not.  It just seems a bit scary.  Like,
		// if a host gets added and then dropped on the same iteration, it will not
		// be in the listbox and I was using that conidition to hopefully flag bugs.

		// Note that we use the m_u32User field of the host structure to store a pointer
		// to the corresponding GUI item.  Since we know these host items get copied
		// from the added list to the main list and then, possibly, to the dropped list
		// this value will remain with the host all the way to the dropped list.
		while (phostslistDropped->GetHead())
			{
			// Get pointer to first host in list
			U32 u32User = phostslistDropped->GetHeadData().m_u32User;

			// We should have already been using this.
			ASSERT(u32User);
			if (u32User)
				{
				// Remove the corresponding GUI list entry from the listbox.
				plb->RemoveItem((RGuiItem*)(u32User));
				// Note that we updated the dialog and will need to re-adjust
				// fields and recompose.
				bRepaginate	= true;
				}

			// Remove first host in list
			phostslistDropped->RemoveHead();
			}

		if (bRepaginate)
			{
			plb->AdjustContents();
			}

		// If there is a selection . . .
		if (plb->GetSel() )
			{
			// Currently, we view this as a choice.  Let's simply set the OK button
			// so it'll be easy to switch back and forth between these two methods.
			RSP_SAFE_GUI_REF_VOID(ms_pguiOk, SetClicked(TRUE) );
			}

		// Empty the added list, which we don't use, but we don't want it to keep
		// growing either, and it's our responsibility to empty it.
		while (phostslistAdded->GetHead())
			phostslistAdded->RemoveHead();
		}
	else
		{
		TRACE("UpdateListBox():  No listbox to update.\n");
		sResult	= -1;
		}

	return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Add a console message.
//
//////////////////////////////////////////////////////////////////////////////
static void AddConsoleMsg(	// Returns nothing.
	bool	bChat,				// In:  true for a chat message, or false for others (like net status).
	const char* pszFrmt,		// In:  sprintf style formatting.
	...)							// In:  Optional arguments based on context of pszFrmt.
	{
	if (ms_plbNetConsole != NULL)
		{
		char szOutput[MAX_STATUS_STR];

		va_list varp;
		va_start(varp, pszFrmt);    
		vsprintf(szOutput, pszFrmt, varp);
		va_end(varp);

		U32	u32TextColor	= ms_plbNetConsole->m_u32TextColor;

		char szMsg[MAX_STATUS_STR];
		// If it's a chat message . . .
		if (bChat)
			{
			// Verbatim is good (already includes name and such).
			strcpy(szMsg, szOutput);
			}
		else
			{
			// Otherwise, add optional prefix.
			sprintf(szMsg, "%s%s", g_pszNetStatusMsgPrefix, szOutput);
			// And use alternate color.
			u32TextColor		= NET_STATUS_MSG_COLOR_INDEX;
			}

		bool	bRepaginate	= false;	// true to repaginate the listbox.

		// If this one will put us over . . .
		if (ms_lNumConsoleEntries >= MAX_CHAT_STRINGS)
			{
			// Get the oldest chat.
			RGuiItem*	pguiOldestConsoleMsg	= ms_plbNetConsole->GetFirst();
			if (pguiOldestConsoleMsg)
				{
				// Remove it.
				ms_plbNetConsole->RemoveItem(pguiOldestConsoleMsg);
				// Repaginate.
				bRepaginate	= true;
				}
			}

		RGuiItem*	pguiConsoleMsg	= ms_plbNetConsole->AddString(szMsg);
		if (pguiConsoleMsg)
			{
			// Another chat.
			ms_lNumConsoleEntries++;
			// Store the old border thickness so we know how much we can reduce
			// these.
			short	sOrigTotalBorderThickness	= pguiConsoleMsg->GetTopLeftBorderThickness() + pguiConsoleMsg->GetBottomRightBorderThickness();
			// No lines.
			pguiConsoleMsg->m_sBorderThickness	= 0;
			// Adjust color.
			pguiConsoleMsg->m_u32TextColor		= u32TextColor;
			// Activate shadow parameters.
			MakeMoreReadable(pguiConsoleMsg);
			// Recreate without space for border lines . . .
			if (pguiConsoleMsg->Create(
				pguiConsoleMsg->m_sX, 
				pguiConsoleMsg->m_sY,
				// We want to keep these as small as possible so we can fit as many as possible.
				// Subtract the border thickness since we don't use it but add 1 for text shadow effect.
				pguiConsoleMsg->m_im.m_sWidth - sOrigTotalBorderThickness + 1,
				pguiConsoleMsg->m_im.m_sHeight - sOrigTotalBorderThickness + 1,
				pguiConsoleMsg->m_im.m_sDepth) == 0)
				{
				// We cannot click on these.
				pguiConsoleMsg->m_sActive	= FALSE;
				pguiConsoleMsg->SetActive(FALSE);
				// Repaginate now, now.
				ms_plbNetConsole->AdjustContents();
				// Ensure the new item is visible.
				ms_plbNetConsole->EnsureVisible(pguiConsoleMsg);
				}
			else
				{
				// This is useless then.
				delete pguiConsoleMsg;
				pguiConsoleMsg	= NULL;
				}
			}
		else
			{
			// If we need to repaginate anyways . . .
			if (bRepaginate)
				{
				// Repaginate now even though we failed to add a new string.
				ms_plbNetConsole->AdjustContents();
				}
			}
		}
	}


//////////////////////////////////////////////////////////////////////////////
//
// Get descriptive text associated with specified error message
//
//////////////////////////////////////////////////////////////////////////////
extern const char* NetErrorText(						// Returns pointer to text
	NetMsg* pmsg)											// In:  Error message
	{
	static char szStaticErrorText[512];
	char* pText = "";
	if (pmsg->msg.nothing.ucType == NetMsg::ERR)
		{
		switch (pmsg->msg.err.error)
			{
			case NetMsg::NoError:
				pText = g_pszNetStat_NoError;
				break;
			case NetMsg::ReceiveError:
				pText = g_pszNetStat_ReceiveError;
				break;
			case NetMsg::InQFullError:
				pText = g_pszNetStat_InQFullError;
				break;
			case NetMsg::OutQFullError:
				pText = g_pszNetStat_OutQFullError;
				break;
			case NetMsg::SendError:
				pText = g_pszNetStat_SendError;
				break;
			case NetMsg::InQReadError:
				pText = g_pszNetStat_InQReadError;
				break;
			case NetMsg::OutQWriteError:
				pText = g_pszNetStat_OutQWriteError;
				break;
			case NetMsg::ConnectionError:
				pText = g_pszNetStat_ConnectionError;
				break;
			case NetMsg::TimeoutError:
				pText = g_pszNetStat_TimeoutError;
				break;
			case NetMsg::ListenError:
				pText = g_pszNetStat_ListenError;
				break;
			case NetMsg::CantConnectError:
				pText = g_pszNetStat_ConnectError;
				break;
			case NetMsg::ConnectTimeoutError:
				pText = g_pszNetStat_ConnectTimeoutError;
				break;
			case NetMsg::ServerVersionMismatchError:
				pText = szStaticErrorText;
				sprintf(pText, g_pszNetStat_ServerVersionMismatchError_lu_lu, pmsg->msg.err.ulParam, CNetMsgr::CurVersionNum & ~CNetMsgr::MacVersionBit);
				break;
			case NetMsg::ClientVersionMismatchError:
				pText = szStaticErrorText;
				sprintf(pText, g_pszNetStat_ClientVersionMismatchError_lu_lu, pmsg->msg.err.ulParam, CNetMsgr::CurVersionNum & ~CNetMsgr::MacVersionBit);
				break;
			case NetMsg::LoginDeniedError:
				pText = g_pszNetStat_LoginDeniedError;
				break;
			case NetMsg::JoinDeniedError:
				pText = g_pszNetStat_JoinDeniedError;
				break;
			case NetMsg::CantOpenPeerSocketError:
				pText = g_pszNetStat_CantOpenPeerSocketError;
				break;
			case NetMsg::ServerPlatformMismatchError:
				pText = g_pszNetStat_ServerPlatformMismatchError;
				break;
			case NetMsg::ClientPlatformMismatchError:
				pText = g_pszNetStat_ClientPlatformMismatchError;
				break;
			default:
				pText = g_pszNetStat_UnknownError;
				break;
			}
		}
	return pText;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the realm filename from the realm title, using the INI.
//
//////////////////////////////////////////////////////////////////////////////
static short GetRealmFileFromRealmTitle(	// Returns 0, if found; non-zero
														// otherwise.
	bool	bCoopLevel,								// In:  true, if a coop level; false, if deathmatch level.
	char*	pszRealmTitle,							// In:  Realm title.
	char* pszRealmFileName,						// Out: Realm filename.
	short sMaxLen)									// In:  Max space available at 
														// pszRealmFileName.
	{
	short	sResult	= 0;	// Assume success.

	RPrefs prefsRealm;
	// Try opening the realms.ini file on the HD path first, if that fails go to the CD
	sResult = prefsRealm.Open(FullPathHD(g_GameSettings.m_pszRealmPrefsFile), "rt");
	if (sResult != 0)
		sResult = prefsRealm.Open(FullPathCD(g_GameSettings.m_pszRealmPrefsFile), "rt");
	if (sResult == 0)
		{
		// Try each realm section until we find the title we're looking for
		// or we find an empty entry.
		// Multiplayer sections are named "RealmNet1, "RealmNet2", etc.
		// Multiplayer realm entry is always "Realm".
		// The title is always "Title".
		short	sRealmNum	= 1;
		char	szRealmTitle[512];
		char	szSection[512];
		bool	bFound	= false;
		do
			{
			// Form section name.
			sprintf(
				szSection, 
				"Realm%sNet%d", 
				bCoopLevel ? "Coop" : "", 
				sRealmNum++);

			// Safety; renitialize.
			szRealmTitle[0]	= '\0';

			// Get the title for this net realm.
			prefsRealm.GetVal(szSection, "Title", "", szRealmTitle);

			// If this is the title of the realm we're looking for . . .
			if (rspStricmp(szRealmTitle, pszRealmTitle) == 0)
				{
				// Found it; get filename.
				// Note that there's no real way to use the sMaxLen.
				prefsRealm.GetVal(szSection, "Realm", "", pszRealmFileName);

				// Done.
				bFound	= true;
				}

			} while (szRealmTitle[0] != '\0' && bFound == false);

		// If we didn't find it . . .
		if (bFound == false)
			{
			// Let the caller know.
			sResult	= 1;
			}
		}

	return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Client has been dropped
//
//////////////////////////////////////////////////////////////////////////////
static void OnDroppedMsg(
	CNetClient*	pnet,			// In:  Network interface.
	NetMsg*		pmsg)			// In:  Dropped msg from client to remove.
	{
	ASSERT(pmsg->msg.nothing.ucType == NetMsg::DROPPED);

	// Print message while player's info is still in our database
	if (pmsg->msg.dropped.id == Net::InvalidID)
		AddConsoleMsg(false, "%s", g_pszClientStat_YouWereDropped);
	else
		AddConsoleMsg(false, g_pszClientStat_SomeoneDropped_s, pnet->GetPlayerName(pmsg->msg.dropped.id));

	// Update number of players displayed.
	RGuiItem*	pguiConnected	= ms_pguiRoot->GetItemFromId(GUI_ID_PLAYERS_STATIC1);
	RSP_SAFE_GUI_REF_VOID(pguiConnected, SetText(g_pszNetDlg_ConnectedPlayers_d, pnet->GetNumPlayers()));
	RSP_SAFE_GUI_REF_VOID(pguiConnected, Compose());
	pguiConnected	= ms_pguiRoot->GetItemFromId(GUI_ID_PLAYERS_STATIC2);
	RSP_SAFE_GUI_REF_VOID(pguiConnected, SetText(g_pszNetDlg_ConnectedPlayers_d, pnet->GetNumPlayers()));
	RSP_SAFE_GUI_REF_VOID(pguiConnected, Compose());

	// Get client's GUI . . .
	RGuiItem* pguiClient = ms_plbPlayers->GetItemFromId( long(pmsg->msg.dropped.id));
	if (pguiClient != NULL)
		{
		// Remove the item.
		ms_plbPlayers->RemoveItem(pguiClient);
		ms_plbPlayers->AdjustContents();
		}
	else
		{
		TRACE("RemoveClient(): We didn't even know of this client!!\n");
		}
	}


//////////////////////////////////////////////////////////////////////////////
//
// Player has joined
//
//////////////////////////////////////////////////////////////////////////////
static short OnJoinedMsg(	// Returns 0 on success.
	CNetClient*	pnet,			// In:  Network interface.
	NetMsg*		pmsg,			// In:  Joined msg from client to add.
	bool			bServer)		// In:  true if in server mode; false if client.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pmsg->msg.nothing.ucType == NetMsg::JOINED);
	
	UCHAR	ucColorIndex	= pmsg->msg.joined.ucColor;
	if (ucColorIndex >= CGameSettings::ms_sNumPlayerColorDescriptions)
		ucColorIndex	= 0;

	// Add info to listbox.
	char	szPlayer[256];
	sprintf(szPlayer, "%s   (%s)", pmsg->msg.joined.acName, CGameSettings::ms_apszPlayerColorDescriptions[ucColorIndex]);

	// Update number of players displayed.
	RGuiItem*	pguiConnected	= ms_pguiRoot->GetItemFromId(GUI_ID_PLAYERS_STATIC1);
	RSP_SAFE_GUI_REF_VOID(pguiConnected, SetText(g_pszNetDlg_ConnectedPlayers_d, pnet->GetNumPlayers()));
	RSP_SAFE_GUI_REF_VOID(pguiConnected, Compose() );
	pguiConnected	= ms_pguiRoot->GetItemFromId(GUI_ID_PLAYERS_STATIC2);
	RSP_SAFE_GUI_REF_VOID(pguiConnected, SetText(g_pszNetDlg_ConnectedPlayers_d, pnet->GetNumPlayers()));
	RSP_SAFE_GUI_REF_VOID(pguiConnected, Compose() );

	RGuiItem* pguiClient = ms_plbPlayers->AddString(szPlayer);
	if (pguiClient != NULL)
		{
		// Let's be able to identify this client by its net ID.
		pguiClient->m_lId	= (long)pmsg->msg.joined.id;

		// Don't allow the user to select these in client mode . . .
		if (bServer == false)
			{
			pguiClient->m_sActive	= FALSE;
			pguiClient->SetActive(FALSE);
			}

		// Activate shadow parameters.
		MakeMoreReadable(pguiClient);
		pguiClient->Compose();

		ms_plbPlayers->AdjustContents();
		ms_plbPlayers->EnsureVisible(pguiClient);
		}
	else
		{
		TRACE("OnJoinedMsg(): ms_plbPlayers->AddString() failed.\n");
		sRes = -1;
		}

	return sRes;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Player has changed info
//
//////////////////////////////////////////////////////////////////////////////
static short OnChangedMsg(	// Returns 0 on success.
	CNetClient*	pnet,			// In:  Network interface.
	NetMsg*		pmsg)			// In:  Changed msg
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pmsg->msg.nothing.ucType == NetMsg::CHANGED);

	TRACE("OnChangedMsg(): Changes are not yet refelected in the gui's!\n");

	return sRes;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Incoming chat message
//
//////////////////////////////////////////////////////////////////////////////
static void OnChatMsg(
	CNetClient*	pnet,				// In:  Network interface.
	NetMsg*		pmsg)				// In:  Chat msg.
	{
	ASSERT(pmsg->msg.nothing.ucType == NetMsg::CHAT);
	AddConsoleMsg(true, "%s", pmsg->msg.chat.acText);
	}


//////////////////////////////////////////////////////////////////////////////
//
// Handles SetupGame message.
//
//////////////////////////////////////////////////////////////////////////////
static void OnSetupGameMsg(
	CNetClient*	pnet,				// In:  Network interface.
	NetMsg*		pmsg)				// In:  Message.
	{
	// Paginate into user displayable single string.
	if (ms_pguiOptions)
		{

		// Check whether this message contains the same realm info as the last such
		// message, if there was one.  In other words, is there new realm info?
		bool bNewRealm = false;
		if (ms_bGotSetupMsg)
			{
			if ((pmsg->msg.setupGame.sRealmNum != ms_sSetupRealmNum) ||
				(rspStricmp(pmsg->msg.setupGame.acRealmFile, ms_szSetupRealmFile) != 0))
				{
				bNewRealm = true;
				}
			}
		else
			{
			bNewRealm = true;
			}

		// If it's new, save it for next time
		if (bNewRealm)
			{
			ms_sSetupRealmNum = pmsg->msg.setupGame.sRealmNum;
			strcpy(ms_szSetupRealmFile, pmsg->msg.setupGame.acRealmFile);
			ms_bGotSetupMsg = true;
			}

		// Determine whether the selected realm is available
		bool bAvailable = false;
		#ifdef ENABLE_PLAY_SPECIFIC_REALMS_ONLY
			// If there's only specific realms available, we simply check for
			// the hardwired values.
			if (pmsg->msg.setupGame.sRealmNum > -1)
				{
				if (pmsg->msg.setupGame.sRealmNum == SPECIFIC_MP_REALM_NUM)
					bAvailable = true;
				}
			else
				{
				if (rspStricmp(pmsg->msg.setupGame.acRealmFile, SPECIFIC_MP_REALM_TEXT) == 0)
					bAvailable = true;
				}
		#else
			// If it's a full version of the game, we still need to see if the specified
			// file is available because the host may be trying to use an add-on realm
			// that wasn't on the original CD.
			if (pmsg->msg.setupGame.sRealmNum > -1)
				{
				// Convert realm number to file name
				char szFile[RSP_MAX_PATH];
				if (Play_GetRealmInfo(
					true,					// Multiplayer
					pmsg->msg.setupGame.sCoopLevels ? true : false,	// Cooperative or Deathmatch levels
					false,				// Not gauntlet
					false,				// Not new single player Add On levels
					pmsg->msg.setupGame.sRealmNum,
					pmsg->msg.setupGame.sDifficulty,
					szFile,
					sizeof(szFile)) == 0)
					{
					// Check if file is available
					if (CRealm::DoesFileExist(szFile))
						bAvailable = true;
					}
				}
			else
				{
				// acRealmFile, due to a bizzarre twist of events, is never a filename
				// but, rather, the title of the realm.  The only way we can get a filename
				// from this title is to scan the NetRealm's in the INI.
				char	szRealmFileName[RSP_MAX_PATH];
				if (GetRealmFileFromRealmTitle(
					pmsg->msg.setupGame.sCoopLevels ? true : false,	// Cooperative or Deathmatch levels
					pmsg->msg.setupGame.acRealmFile,
					szRealmFileName,
					sizeof(szRealmFileName) ) == 0)
					{
					// Check if file is available
					if (CRealm::DoesFileExist(szRealmFileName))
						bAvailable = true;
					}
				}
		#endif

		// If level is available, display info about it.  Otherwise, say it isn't available!
		if (bAvailable)
			{
			char	szCoop[256] = "";
			if (pmsg->msg.setupGame.sCoopLevels != 0)
				{
				strcpy(szCoop, "Cooperative ");
				}

			char	szRealm[256];
			if (pmsg->msg.setupGame.sRealmNum > -1)
				{
				// Use number.
				sprintf(szRealm, "Level %hd", pmsg->msg.setupGame.sRealmNum);
				}
			else
				{
				// Use string.
				sprintf(szRealm, "\"%s\"", pmsg->msg.setupGame.acRealmFile);
				}

			char	szPlayMode[256];
			if (pmsg->msg.setupGame.sCoopMode)
				{
				strcpy(szPlayMode, "Cooperative");
				}
			else
				{
				strcpy(szPlayMode, "Deathmatch");
				}

			char	szTimeLimit[256];
			if (pmsg->msg.setupGame.sTimeLimit > 0)
				{
				sprintf(szTimeLimit, "a time limit of %hd", pmsg->msg.setupGame.sTimeLimit);
				}
			else
				{
				sprintf(szTimeLimit, "no time limit");
				}

			char	szKillLimit[256];
			if (pmsg->msg.setupGame.sKillLimit > 0)
				{
				sprintf(szKillLimit, "a kill limit of %hd", pmsg->msg.setupGame.sKillLimit);
				}
			else
				{
				sprintf(szKillLimit, "no kill limit");
				}

			ms_pguiOptions->SetText(
				"Host has selected %s%s in %s mode on difficulty %hd with %s and %s.",
				szCoop,
				szRealm,
				szPlayMode,
				pmsg->msg.setupGame.sDifficulty,
				szTimeLimit,
				szKillLimit);
			}
		else
			{
			// Tell the local user that the selected level is not available
			ms_pguiOptions->SetText("Host has selected a level you don't have!");

			// Send a text message to all players (we'd like to send it to just the host,
			// but we don't support that, and maybe it's nice that the other players see
			// it, too).  The message complains that the selected level is not available.
			// Note that we only send this when the realm info has changed, so as to avoid
			// sending an endless stream of the same messages over and over.
			if (bNewRealm)
				{
				char szTmp[1024];
				#ifdef ENABLE_PLAY_SPECIFIC_REALMS_ONLY
					sprintf(szTmp, "%s\"%s\" only has the \""SPECIFIC_MP_REALM_TEXT"\" hood -- please pick it!",
						g_pszNetStatusMsgPrefix,
						pnet->GetPlayerName(pnet->GetID()));
					pnet->SendText(szTmp);
				#else
					sprintf(szTmp, "%s\"%s\" does not have the selected hood -- please pick another one!",
						g_pszNetStatusMsgPrefix,
						pnet->GetPlayerName(pnet->GetID()));
					pnet->SendText(szTmp);
				#endif
				}
			}

		// Compose options GUI with word wrap preserving original word wrap status.
		ComposeOptions();
		}
	}


//////////////////////////////////////////////////////////////////////////////
//
// Explain the fact that the procol the user chose is not supported
//
//////////////////////////////////////////////////////////////////////////////
void ProtoNotSupported(void)
	{
	// Check if there's only one protocol or more than one
	if (RSocket::FirstProtocol + 1 == RSocket::NumProtocols)
		{
		// Only one protocol
		rspMsgBox(
			RSP_MB_BUT_OK | RSP_MB_ICN_INFO,
			g_pszAppName,
			g_pszNetOnlyProtocolUnsupported_s,
			RSocket::GetProtoName());
		}
	else
		{
		// More than one protocol
		rspMsgBox(
			RSP_MB_BUT_OK | RSP_MB_ICN_INFO,
			g_pszAppName,
			g_pszNetProtocolUnsupported_s,
			RSocket::GetProtoName());
		}
	}


//////////////////////////////////////////////////////////////////////////////
//
// Do network game dialog
//
//////////////////////////////////////////////////////////////////////////////
extern short DoNetGameDialog(							// Returns 0 if successfull, non-zero otherwise.
	CNetClient*	pclient,									// I/O: Client interface
	bool bBrowse,											// In:  Whether to browse (true) or connect (false)
	CNetServer*	pserver,									// I/O: Server interface or NULL if not server
	NetMsg* pmsgOut)										// Out: NetMsg::NOTHING or NetMsg::START_GAME
	{
	ASSERT(pclient != NULL);

	short	sResult = 0;									// Assume success.

	// Under Win95 with DirectX, certain problems have come up due to a
	// combination of our hogging the CPU and DirectX adding to that hogging,
	// and us taking over the screen via DirectX, all of which is a problem
	// when the dialup-networking dialog tries to display itself.  In an
	// attempt to alleviate some of this, we go to a lower-level of CPU hogging
	// during the network dialogs, and restore the level afterwards.
	rspSetDoSystemMode(RSP_DOSYSTEM_TOLERATEOS);

	// Init globals
	ms_pguiRoot			= NULL;
	ms_pguiOk			= NULL;
	ms_pguiCancel		= NULL;
	ms_plbPlayers		= NULL;
	ms_plbNetConsole	= NULL;
	ms_pguiChatText	= NULL;
	ms_pguiChatSend	= NULL;
	ms_plbLevelBrowse	= NULL;
	ms_pmbCoopLevels	= NULL;
	ms_pmbCoopMode		= NULL;
	ms_pguiRetry		= NULL;
	ms_lWatchdogTime	= 0;
	ms_bNetBlockingAbort = false;

	// We haven't received a setup msg yet
	ms_bGotSetupMsg = false;

	ClearNetProb();

	// Make sure vars with built in flagging get reflected properly.


	// No time limit is flagged as 0 or negative
	if (g_GameSettings.m_sHostTimeLimit > 0)
		{
		ms_bTimeLimit	= true;
		}
	else
		{
		ms_bTimeLimit	= false;
		g_GameSettings.m_sHostTimeLimit	= -g_GameSettings.m_sHostTimeLimit;
		}
	
	// No kill limit is flagged as 0 or negative
	if (g_GameSettings.m_sHostKillLimit > 0)
		{
		ms_bKillLimit	= true;
		}
	else
		{
		ms_bKillLimit	= false;
		g_GameSettings.m_sHostKillLimit	= -g_GameSettings.m_sHostKillLimit;
		}

	// Save current mouse cursor level and then force it to be visible
	short sCursorLevel = rspGetMouseCursorShowLevel();
	rspSetMouseCursorShowLevel(1);

	// Clear any events that might be in the queue
	rspClearAllInputEvents();

	// *************************** TEMP
//	bBrowse	= true;
	// *************************** TEMP

	// If we're browsing, this loop continues until the user aborts or starts.
	// If we're not browsing, this loop only goes through once.
	bool bAbort = false;
	bool bStart = false;
	do	{
		// Call this periodically to let it know we're not locked up
		NetBlockingWatchdog();

		// Default to returning a NetMsg::NOTHING
		pmsgOut->msg.nothing.ucType = NetMsg::NOTHING;

		// If there's a server, start it up
		if (pserver)
			sResult = pserver->Startup(g_GameSettings.m_usServerPort, g_GameSettings.m_szHostName,	&NetBlockingCallback);
		if (sResult == 0)
			{

			// Host address
			RSocket::Address addressHost;

			// If we're browsing, do it before the client dialog since it uses a dialog of its own
			if (bBrowse)
				sResult = BrowseForHost(pserver, &addressHost);
			if (sResult == 0)
				{

				// Setup dialog in client or server mode, as appropriate
				sResult	= SetupDlg(pserver ? FullPathHD(SERVER_GUI) : FullPathVD(CLIENT_GUI), pserver ? DLG_SERVER : DLG_CLIENT);
				if (sResult == 0)
					{
					// Display "startup" messages
					if (pserver)
						AddConsoleMsg(false, "%s", g_pszServerStat_Startup);
					else
						AddConsoleMsg(false, g_pszClientStat_Startup);

					// Update the dialog so it shows up on the screen.  We ignore the
					// return since we aren't ready to deal with it here.  This seems
					// okay since the dialog has just shown up, so nobody (except Jeff)
					// could possibly hit a key or mouse button on the first frame.
					UpdateDialog(ms_pguiRoot, true);

					// If we didn't browse for a host, we still need to find one
					if (!bBrowse)
						{
						if (pserver)
							{
							// Try to find the local server
							sResult = BrowseForSelf(pserver, &addressHost);
							}
						else
							{
							// The local client tries to find a remote server
							sResult = FindSpecificSystem(&addressHost);
							}
						}
					if (sResult == 0)
						{

						// This loop is used for when the user hits the "RETRY" button on the client dialog
						bool bRetry = false;
						do {
							// Clear retry flag each time so we don't accidentally get caught in a loop
							bRetry = false;

							// Startup client
							pclient->Startup(&NetBlockingCallback);

							// Start the (asynchronous) process of joining the specified host.  We
							// specifically check for the "protocol not supported" error and handle
							// that as a special case, because users are likely to screw that up.
							// Any other errors will be discovered via our normal GetMsg() loop.
							sResult = pclient->StartJoinProcess(
								&addressHost,
								g_GameSettings.m_szPlayerName,
								g_GameSettings.m_sPlayerColorIndex,
								0,
								g_GameSettings.m_sNetBandwidth);
							if (sResult != RSocket::errNotSupported)
								{

								//------------------------------------------------------------------------------
								// This may appear to be a rather bizarre loop, and maybe it is, but it seemed
								// like a really good idea at the time...
								//
								// It handles three primary tasks.  First, if this machine is a server, it does
								// all the server stuff.  Then, it does the client stuff (remember that a server
								// is always a client, but a client is not necessarily a server).  Finally, it
								// does the dialog and user-input stuff, which again is slightly different
								// depending on whether it's a client or a server.
								//
								// The alternative approach was to have separate functions to handle the client
								// and server ends.  It was originally that way, but there was so much
								// similarity between the two that it seemed better to merge them, thereby
								// avoiding having to maintain two very similar sets of code.
								//
								// A key point about this loop is that you should NEVER use a 'break' to get
								// out of the loop.  In fact, you should rarely exit the loop in any kind of
								// direct manner.  Instead, you should always try to wait until the client
								// and server (if any) have been allowed to "gracefully" finish whatever it
								// is they need to get done.
								//
								// Note that if the user clicks the "ABORT" button in the dialog, we don't
								// immediately end the loop, either.  We simply set the flags and hope the
								// loop will end itself soon enough.  If, after a certain amount of time, the
								// loop hasn't ended, the user can click "ABORT" again to end, and this time
								// we end immediately.  At some point, this should perhaps be replaced by a
								// timer which automatically ends the loop after a certain amount of time.
								//------------------------------------------------------------------------------
								bool bEndClientCleanly = false;
								bool bEndServerCleanly = false;
								bool bServerDone = pserver ? false : true;
								bool bClientDone = false;
								bool bUserAbortNow = false;
								long lCancelDelay;
								DLG_ACTION action;
								NetMsg msg;
								while (!(bClientDone && bServerDone && (bAbort || bStart || bRetry)) && !bUserAbortNow)
 									{
									// Always and forever
									UpdateSystem();

									// Call this periodically to let it know we're not locked up
									NetBlockingWatchdog();

									//------------------------------------------------------------------------------
									// Do dialog and any other user-input stuff
									//------------------------------------------------------------------------------

									// Update GUI
									action = UpdateDialog(ms_pguiRoot, true);

									// If user wants to quit, simulate a cancel.  Note that this is NOT an event
									// but a flag, so once it starts, we'll continue to get CANCEL actions each
									// time we get to this point.  This is exactly what we want, since this type
									// of quit indicates that the user doesn't want to wait around to quit.
									if (rspGetQuitStatus() || NetBlockingWasAborted())
										action = DLG_CANCEL;

									// Handle action
									switch (action)
										{
										// Nothing to do
										case DLG_NOTHING:
											break;

										// OK was pressed.  This only exists on server dialog!  Note that we don't
										// allow OK to be used if either it or Cancel were already used.
										case DLG_OK:
											// Must be server. . .
											if (pserver)
												{
												// Make sure neither OK nor Cancel were pressed, and that we have at least 1 client
												if (!bStart && !bAbort && pserver->GetNumberOfClients())
													{
													// Download GUI vals.
													DownloadLinks(ms_aglServerLinkage, ms_pguiRoot);

													// Feedback
													PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);
													AddConsoleMsg(false, "%s", g_pszNetStat_Starting);

													// Get pointer to game selection GUI
													RGuiItem* pguiSel = RSP_SAFE_GUI_REF(ms_plbLevelBrowse, GetSel());

													// Start game using specified settings
													pserver->StartGame(
														pclient->GetID(),
														#ifdef ENABLE_PLAY_SPECIFIC_REALMS_ONLY
															-1,
															SPECIFIC_MP_REALM_FILE,
														#else
															RSP_SAFE_GUI_REF(pguiSel, m_lId),
															"",
														#endif
														g_GameSettings.m_sDifficulty,
														g_GameSettings.m_sHostRejuvenate,
														ms_bTimeLimit ? g_GameSettings.m_sHostTimeLimit : -g_GameSettings.m_sHostTimeLimit,
														ms_bKillLimit ? g_GameSettings.m_sHostKillLimit : -g_GameSettings.m_sHostKillLimit,
														ms_bCoopLevels ? 1 : 0,
														ms_bCoopMode ? 1 : 0,
														g_GameSettings.m_sNetTimePerFrame,
														g_GameSettings.m_sNetMaxFrameLag);

													// End server cleanly (client will end when it gets start message)
													bEndServerCleanly = true;
													}
												}
											break;

										// Cancel was pressed.  Note that in server mode, we allow cancel to be
										// pressed even after OK was pressed.  We merely send another message
										// to the clients saying to abort the game that was already started.
										case DLG_CANCEL:
											// If cancel was already pressed, we ignore additional cancel's until
											// the timer has expired, at which point they can hit cancel again.
											// The idea is to give the cancel process time to finish cleanly.
											if (bAbort)
												{
												if ((rspGetMilliseconds() - lCancelDelay) > CANCEL_DELAY_TIME)
													{
													// Feedback sound
													PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

													// User has aborted, exit immediately
													bUserAbortNow = true;
													}
												}
											else
												{
												// Pressed cancel, so set flag and init delay timer
												bAbort = true;
												lCancelDelay = rspGetMilliseconds();

												// Feedback
												AddConsoleMsg(false, "%s", g_pszNetStat_Aborting);
												PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

												if (pserver)
													{
													// Download GUI vals anyways for consistency.
													DownloadLinks(ms_aglServerLinkage, ms_pguiRoot);

													// Tell players to abort game
													pserver->AbortGame(NetMsg::UserAbortedGame);
													}
												else
													{
													// Drop out of game
													pclient->Drop();

													// Start with a clean slate (no RETRY button because user selected ABORT)
													CleanClientDlg(pclient, false);
													}

												// End server and client cleanly
												bEndServerCleanly = true;
												bEndClientCleanly = true;
												}
											break;

										// Chat was pressed.  We don't allow this if OK or Cancel were pressed.
										case DLG_CHAT:
											// Make sure neither Ok nor Cancel were pressed
											if (!bStart && !bAbort)
												{
												// Make sure text field exists
												if (ms_pguiChatText != NULL)
													{
													// See if there's any text
													if (ms_pguiChatText->m_szText[0] != '\0')
														{
														// Sent chat
														pclient->SendChat(ms_pguiChatText->m_szText);

														// Clear text
														ms_pguiChatText->SetText("");
														ms_pguiChatText->Compose();
														}
													}
												}
											break;

										// Disconnect player was pressed.  We don't allow this if OK or Cancel were pressed.
										case DLG_DISCONNECT_PLAYER:
											// Make sure neither Ok nor Cancel were pressed
											if (!bStart && !bAbort)
												{
												// If there's a selection . . .
												RGuiItem*	pguiPlayerSel	= ms_plbPlayers->GetSel();
												if (pguiPlayerSel)
													{
													Net::ID id = (UCHAR)pguiPlayerSel->m_lId;
													// Don't be a moron -- don't drop yourself
													if (id == pclient->GetID())
														{
														// Tell user he can't drop himself
														AddConsoleMsg(false, "%s", g_pszServerStat_CantDropSelf);
														}
													else
														{
														// Drop player (must get his name BEFORE he is dropped!)
														AddConsoleMsg(false, g_pszNetStat_AttemptToDrop_s, pserver->GetPlayerName(id));
														pserver->DropClient(id);
														}
													}
												}
											break;

										// Send an options update.  We don't allow this if OK or Cancel were pressed.
										case DLG_OPTIONS_UPDATED:
											if (pserver && !bStart && !bAbort)
												{
												// Get current settings into their variables.
												DownloadLinks(ms_aglServerLinkage, ms_pguiRoot);

												// Get pointer to game selection GUI
												RGuiItem* pguiSel = RSP_SAFE_GUI_REF(ms_plbLevelBrowse, GetSel());

												// Setup game
												pserver->SetupGame(
													#ifdef ENABLE_PLAY_SPECIFIC_REALMS_ONLY
														-1,
														SPECIFIC_MP_REALM_TEXT,
													#else
														pguiSel ? -1                : 0,
														pguiSel ? pguiSel->m_szText : "",
													#endif
													g_GameSettings.m_sDifficulty,
													g_GameSettings.m_sHostRejuvenate,
													ms_bTimeLimit ? g_GameSettings.m_sHostTimeLimit : -g_GameSettings.m_sHostTimeLimit,
													ms_bKillLimit ? g_GameSettings.m_sHostKillLimit : -g_GameSettings.m_sHostKillLimit,
													ms_bCoopLevels ? 1 : 0,
													ms_bCoopMode ? 1 : 0);

												}
											break;

										case DLG_RETRY:
											if (!bStart && !bAbort)
												{
												// Clean the slate (without RETRY since they just hit it and may not need it again)
												CleanClientDlg(pclient, false);

												// Feedback
												PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);
												AddConsoleMsg(false, "%s", g_pszClientStat_Retrying);
												AddConsoleMsg(false, g_pszClientStat_Startup);

												// End server and client cleanly
												bEndServerCleanly = true;
												bEndClientCleanly = true;

												// Set retry flag
												bRetry = true;
												}
											break;

										default:
											TRACE("DoNetGameDialog(): Unknown dialog action!\n");
											break;
										}

									//------------------------------------------------------------------------------
									// Do server stuff.
									//
									// Note that bServerDone should rarely be set to true directly.  Instead, set
									// bEndServerCleanly to true, which will try to end the server cleanly, and will
									// eventually result in bServerDone being set to true.
									//
									// It's okay to ignore incoming messages when trying to end because they will
									// still be received and buffered, they just won't be processed here.  Instead,
									// they'll be processed in the game loop.  Of course, if the game never actually
									// gets started they won't ever be processed, but then it doesn't matter.
									//------------------------------------------------------------------------------

									if (pserver && !bServerDone)
										{
										pserver->Update();

										// Check if we're trying to end
										if (bEndServerCleanly)
											{
											// If there isn't anything more to send, then we're done
											if (!pserver->IsMoreToSend())
												bServerDone = true;
											}
										else
											{
											// Process messages
											pserver->GetMsg(&msg);
											switch(msg.msg.nothing.ucType)
												{
												case NetMsg::ERR:
													if (msg.ucSenderID == Net::InvalidID)
														{
														// A server occurred occurred
														AddConsoleMsg(false, NetErrorText(&msg));

														// Play a sound of badness
														PlaySample(g_smidEmptyWeapon, SampleMaster::UserFeedBack);

														switch (msg.msg.nothing.ucType)
															{
															// These messages are NOT so horrible and don't
															// require us to clean up and be done.
															// They are merely notifications.
															case NetMsg::ServerVersionMismatchError:
															case NetMsg::ServerPlatformMismatchError:
																break;
															
															// These messages are so horrible and don't
															// require us to clean up and be done.
															default:
																// End server and client cleanly
																bEndServerCleanly = true;
																bEndClientCleanly = true;
																break;
															}
														}
													else if (msg.ucSenderID == pclient->GetID())
														{
														// The error occurred on the local client.  We registered the
														// local client with the server, so it will have aborted the game.
														AddConsoleMsg(false, NetErrorText(&msg));

														// Play a sound of badness
														PlaySample(g_smidEmptyWeapon, SampleMaster::UserFeedBack);

														// End server and client cleanly
														bEndServerCleanly = true;
														bEndClientCleanly = true;
														}
													else
														{
														// The error occurred on a remote client, so the server will
														// have dropped that client.
														AddConsoleMsg(false, g_pszServerStat_PlayerErr);
														}
													break;

												default:
													break;
												}
											}
										}

									//------------------------------------------------------------------------------
									// Do client stuff
									//
									// Note that bClientDone should rarely be set to true directly.  Instead, set
									// bEndClientCleanly to true, which will try to end the client cleanly, and will
									// eventually result in bClientDone being set to true.
									//------------------------------------------------------------------------------

									if (!bClientDone)
										{
										pclient->Update();

										// Check if we're trying to end
										if (bEndClientCleanly)
											{
											// If there isn't anything more to send, then we're done
											if (!pclient->IsMoreToSend())
												bClientDone = true;
											}
										else
											{
											// Assume no problems
											short sProblem = 0;

											// Process messages from server
											pclient->GetMsg(&msg);
											switch(msg.msg.nothing.ucType)
												{
												case NetMsg::NOTHING:
													break;

												case NetMsg::ERR:
													AddConsoleMsg(false, NetErrorText(&msg));

													// Attempt to drop nicely
													pclient->Drop();

													// End server and client cleanly
													bEndServerCleanly = true;
													bEndClientCleanly = true;

													// Play a sound of badness and clean the slate (and show RETRY button)
													PlaySample(g_smidEmptyWeapon, SampleMaster::UserFeedBack);
													CleanClientDlg(pclient, true);
													break;

												case NetMsg::STAT:
													switch (msg.msg.stat.status)
														{
														case NetMsg::Opened:
															AddConsoleMsg(false, "%s", g_pszClientStat_Opened);
															break;

														case NetMsg::Connected:
															AddConsoleMsg(false, "%s", g_pszClientStat_Connected);
															break;

														case NetMsg::LoginAccepted:
															// If we are the server's local client, register ourself so
															// that it realizes that if there's an error on our connection,
															// it will abort the whole game.
															if (pserver)
																pserver->SetLocalClientID(pclient->GetID());
															AddConsoleMsg(false, g_pszClientStat_LoginAccepted_hd, (short)pclient->GetID());
															break;

														case NetMsg::JoinAccepted:
															AddConsoleMsg(false, "%s", g_pszClientStat_JoinAccepted);
															break;

														default:
															break;
														}
													break;

												case NetMsg::JOINED:
													if (OnJoinedMsg(pclient, &msg, pserver ? true : false) != 0)
														{
														// A program error occurred, so tell them something vague
														AddConsoleMsg(false, g_pszNetStat_ProgramError);

														// Attempt to drop nicely
														pclient->Drop();

														// End server and client cleanly
														bEndServerCleanly = true;
														bEndClientCleanly = true;

														// Play a sound of badness and clean the slate (and show RETRY button)
														PlaySample(g_smidEmptyWeapon, SampleMaster::UserFeedBack);
														CleanClientDlg(pclient, true);
														}
													break;
												
												case NetMsg::DROPPED:
													OnDroppedMsg(pclient, &msg);
													// If we were dropped, end immediately
													if (msg.msg.dropped.id == Net::InvalidID)
														{
														// Play a sound of badness and clean the slate  (and show RETRY button)
														PlaySample(g_smidEmptyWeapon, SampleMaster::UserFeedBack);
														CleanClientDlg(pclient, true);

														bClientDone = true;
														}
													break;

												case NetMsg::CHAT:
													OnChatMsg(pclient, &msg);
													break;

												case NetMsg::SETUP_GAME:
													OnSetupGameMsg(pclient, &msg);
													break;

												case NetMsg::START_GAME:
													// Return this message to caller
													*pmsgOut = msg;

													// Game has been started
													bStart = true;

													// End immediately (no need to wait)
													bClientDone = true;
													break;

												case NetMsg::ABORT_GAME:
													AddConsoleMsg(false, "%s", g_pszClientStat_ServerAborted);

													// Play a sound of badness and clean the slate (and show RETRY button)
													PlaySample(g_smidEmptyWeapon, SampleMaster::UserFeedBack);
													CleanClientDlg(pclient, true);

													// End immediately (no need to wait)
													bClientDone = true;
													break;

												default:
													AddConsoleMsg(false, "%s", g_pszNetStat_UnhandledMsg);
													break;
												}
											}
										}

									} // while()
								}
							else
								{
								// Display msg box that says the protocol is not supported
								ProtoNotSupported();
								}

							// If we're not starting, shutdown client
							if (!bStart)
								pclient->Shutdown();

							} while (bRetry && !sResult);
						}

					// Get rid of dialog
					DlgBeGone();

					// Call this periodically to let it know we're not locked up
					NetBlockingWatchdog();
					}
				else
					{
					TRACE("DoNetGameDialog(): Failed to setup dialog.\n");
					}
				}

			// If not starting, shutdown server
			if (!bStart && pserver)
				pserver->Shutdown();
			}
		else
			{
			// Handle the "protocol not supported" error as a special case because users
			// are likely to screw that up.
			if (sResult == RSocket::errNotSupported)
				{
				// Display msg box that says the protocol is not supported
				ProtoNotSupported();
				}
			}

		} while (!sResult && (bBrowse && !bAbort && !bStart));

	// Call this one last time on the way out
	NetBlockingWatchdog();

	// Make sure vars with built in flagging get reflected properly.
	if (ms_bTimeLimit == false)
		{
		// Flag no time limit as a negative or zero.
		g_GameSettings.m_sHostTimeLimit	= -g_GameSettings.m_sHostTimeLimit;
		}
	
	if (ms_bKillLimit == false)
		{
		// Flag no kill limit as a negative or zero.
		g_GameSettings.m_sHostKillLimit	= -g_GameSettings.m_sHostKillLimit;
		}

	// Clear any events that might be in the queue
	rspClearAllInputEvents();

	// Restore mouse cursor show level
	rspSetMouseCursorShowLevel(sCursorLevel);


	// Go back to normal CPU mode
	#if defined(_DEBUG)
		// Wake CPU.
		rspSetDoSystemMode(RSP_DOSYSTEM_TOLERATEOS);
	#else
		// Return CPU to us.
		rspSetDoSystemMode(RSP_DOSYSTEM_HOGCPU);
	#endif

	return sResult;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Browse for a host.
//
// This is normally only used in client-only mode, but it is easier to test
// and debug if we can also use it in client-server mode.  That's why this
// takes a pserver pointer.
//
//////////////////////////////////////////////////////////////////////////////
static short BrowseForHost(
	CNetServer*	pserver,									// I/O: Server interface or NULL if none
	RSocket::Address* paddress)						// Out: Address returned here (if successfull)
	{
	short sResult = 0;

	// Start with empty list of hosts
	CNetBrowse::Hosts hostsAll;
	CNetBrowse::Hosts hostsAdded;
	CNetBrowse::Hosts hostsDropped;

	NetBlockingWatchdog();

	// Create browser and start it up
	CNetBrowse browse;
	sResult = browse.Startup(g_GameSettings.m_usServerPort, &NetBlockingCallback);
	if (sResult == 0)
		{
		sResult = SetupDlg(FullPathVD(BROWSER_GUI), DLG_BROWSER);
		if (sResult == 0)
			{
			// Since we don't let RProcessGUI draw cleaned up the screen, we should.
			rspUpdateDisplay();

			DLG_ACTION action	= DLG_NOTHING;
			// Loop until error, user abort, or user choice . . .
			while ((sResult == 0) && action != DLG_OK && action != DLG_CANCEL)
				{
				// Do default processing.
				UpdateSystem();
				// Update watchdog timer for net blocking.
				NetBlockingWatchdog();
				// Update server interface, if available.
				if (pserver)
					pserver->Update();

				// Check for new or lost host games.
				browse.Update(&hostsAll, &hostsAdded, &hostsDropped);

				// Update listbox via added and dropped hosts lists.
				sResult = UpdateListBox(ms_plbHostBrowse, &hostsAll, &hostsAdded, &hostsDropped);
				if (sResult == 0)
					{
					// Update the dialog (user input and GUI output).
					action	= UpdateDialog(ms_pguiRoot, true);
					}

				// If quitting . . .
				if (rspGetQuitStatus() != FALSE)
					sResult = 1;
				}

			// If no error . . .
			if (sResult == 0)
				{
				switch (action)
					{
					case DLG_OK:
						{
						// Play feedback sound
						PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

						// Get selection . . .
						RGuiItem*	pguiSel	= ms_plbHostBrowse->GetSel();
						// If there's no selection . . .
						if (pguiSel == NULL)
							{
							// Take the first.
							pguiSel	= ms_plbHostBrowse->GetFirst();
							}

						// If tehre's a selection . . .
						if (pguiSel)
							{
							// Get corresponding host.
							CNetBrowse::CHost* phost = (CNetBrowse::CHost*)(pguiSel->m_ulUserData);
							ASSERT(phost);
							// Get host address.
							*paddress = phost->m_address;
							// Success.
							}
						else
							{
							TRACE("BrowseForHost():  No host selected.\n");
							sResult	= -1;
							}
						break;
						}

					case DLG_CANCEL:
						// Play feedback sound
						PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

						// Cancelled.
						sResult	= 1;
						break;

					default:
						TRACE("BrowseForHost(): Unknown action.\n");
						sResult	= 1;
						break;
					}
				}

			// Done with dialog.
			DlgBeGone();
			}

		// Stop browsing
		NetBlockingWatchdog();
		browse.Shutdown();
		}
	else
		{
		// Handle the "protocol not supported" error as a special case because users
		// are likely to screw that up.
		if (sResult == RSocket::errNotSupported)
			{
			// Display msg box that says the protocol is not supported
			ProtoNotSupported();
			}
		}

	return sResult;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Try to connect to specified host
//
//////////////////////////////////////////////////////////////////////////////
static short FindSpecificSystem(
	RSocket::Address* paddress)						// Out: Address returned here (if successfull)
	{
	short sResult = 0;

	// Lookup the specified host (by name or dotted address) and port
	sResult = CNetBrowse::LookupHost(
		g_GameSettings.m_szServerName,
		g_GameSettings.m_usServerPort, 
		paddress);
	if (sResult == 0)
		{
		// Success!
		}
	else
		{
		rspMsgBox(
			RSP_MB_BUT_OK | RSP_MB_ICN_INFO,
			g_pszAppName,
			"The specified computer ('%s') could not be found.  Please verify that the specified name or address is correct.",
			g_GameSettings.m_szServerName);
		}

	return sResult;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Try to find the local server.  This is necessary because there isn't any
// foolproof method by which to simply lookup the local computer's address.
// Instead, we browse for ourselves, just like any other client would.
//
//////////////////////////////////////////////////////////////////////////////
static short BrowseForSelf(
	CNetServer*	pserver,									// I/O: Server interface
	RSocket::Address* paddress)						// Out: Address returned here (if successfull)
	{
	ASSERT(pserver);

	short sResult = 0;

	// Start with empty list of hosts
	CNetBrowse::Hosts hostsAll;
	CNetBrowse::Hosts hostsAdded;
	CNetBrowse::Hosts hostsDropped;

	NetBlockingWatchdog();

	// Create browser and start it up
	bool bFoundSelf = false;
	CNetBrowse browse;
	sResult = browse.Startup(g_GameSettings.m_usServerPort, &NetBlockingCallback);
	if (sResult == 0)
		{
		// Wait for our own broadcast  
		long lTime = rspGetMilliseconds() + Net::BroadcastDropTime;
		while (!bFoundSelf && (rspGetMilliseconds() < lTime))
			{
			UpdateSystem();
			NetBlockingWatchdog();
			pserver->Update();
			browse.Update(&hostsAll, &hostsAdded, &hostsDropped);

			// Try to find ourself in the list
			for (CNetBrowse::Hosts::Pointer p = hostsAll.GetHead(); p; p = hostsAll.GetNext(p))
				{
				CNetBrowse::CHost* phost;
				phost = &hostsAll.GetData(p);
				if ((strcmp(phost->m_acName, pserver->GetHostName()) == 0) && (phost->m_lMagic == pserver->GetHostMagic()))
					{
					// Return this host's address
					*paddress = phost->m_address;
					bFoundSelf = true;
					break;
					}
				}
			}

		// If we didn't find ourself, set the error flag
		if (!bFoundSelf)
			{
			sResult = -1;
			TRACE("BrowseForSelf(): Couldn't find myself!\n");
			}

		// Stop browsing
		NetBlockingWatchdog();
		browse.Shutdown();
		}


	// If this failed, put up a msgbox
	if (sResult != 0)
		{
		rspMsgBox(
			RSP_MB_BUT_OK | RSP_MB_ICN_INFO,
			g_pszAppName,
			"This computer's network address could not be determined.  If this is a multi-homed host, you may need to disable all but one address.");
		}

	return sResult;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Net blocking watchdog.  Call this periodically to let the watchdog know
// that the program hasn't locked up.
//
//////////////////////////////////////////////////////////////////////////////
extern void NetBlockingWatchdog(void)
	{
	// Reset timer to current time
	ms_lWatchdogTime = rspGetMilliseconds();
	
	// If net blocking had expired, then clear it so the net prob gui goes
	// away (since we got here, we're obviously not blocking anymore)
	if (m_bNetWatchdogExpired)
		{
		// Clear flag
		m_bNetWatchdogExpired = false;

		// Erase net prob gui
		RGuiItem* ptxt	= GetNetProbGUI();
		if (ptxt)
			{
			// This will PROBABLY work, but it does assume that something else
			// was drawn to the buffer after the last time we drew the net gui.
			// If nothing was drawn to the buffer, the gui will still be there!
			rspUpdateDisplay(ptxt->m_sX, ptxt->m_sY, ptxt->m_im.m_sWidth, ptxt->m_im.m_sHeight);
			}
		}
	}


//////////////////////////////////////////////////////////////////////////////
//
// Check if a blocked network operation was aborted.
//
//////////////////////////////////////////////////////////////////////////////
extern bool NetBlockingWasAborted(void)
	{
	return ms_bNetBlockingAbort;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Net blocking callback
//
//////////////////////////////////////////////////////////////////////////////
static short NetBlockingCallback(void)				// Returns 0 to continue normally, 1 to abort
	{
	// We only need to grab this ptr once (it is guaranteed not to move).
	// Once we have this pointer to the Key status array, we can use it to
	// check on a key's status.  The important thing would be NOT to clear
	// any key's status so we don't affect the input module (input.cpp) or
	// the play loop (play.cpp:PlayRealm()).
	static U8*	pau8KeyStatus	= rspGetKeyStatusArray();

	// Assume we won't abort
	short	sAbort = 0;

	// It's always a good idea to do this
	UpdateSystem();

	// System-specific quit always aborts immediately
	if (rspGetQuitStatus())
		sAbort = 1;

	// If the watchdog hasn't already expired, then check it.  If it has
	// already expired, then we need to see if the user hit the abort key.
	if (!m_bNetWatchdogExpired)
		{
		// If the watchdog function hasn't been called in a too-long time, then we
		// assume the network stuff is blocking (although it could be that someone
		// forgot to call the watchdog function) and we display the net problem gui.
		if ((rspGetMilliseconds() - ms_lWatchdogTime) > g_GameSettings.m_lNetMaxBlockingTime)
			{
			// Set flag to indicate that there is a problem.  This servers two
			// purposes: (1) it let's us know that the net prob gui is being
			// displayed and we should check for the abort key, and (2) it let's
			// other parts of the program determine whether or not to draw the
			// net prob gui on top of whatever else is being drawn to the screen.
			m_bNetWatchdogExpired	= true;
			}
		}

	// If watchdog is expired, draw the net prob gui and check for an abort key press
	if (m_bNetWatchdogExpired)
		{
		// Display net problem gui
		RGuiItem* ptxt	= GetNetProbGUI();
		if (ptxt)
			{
			// Set to display general error and abort instructions
			ptxt->SetText("%s", g_pszNetProb_General);
			ptxt->Compose();
			
			// Center it so it gets erased by whatever menu or dialog is being displayed
			ptxt->Move(
				(g_pimScreenBuf->m_sWidth / 2) - (ptxt->m_im.m_sWidth / 2),
				(g_pimScreenBuf->m_sHeight / 2) - (ptxt->m_im.m_sHeight / 2));

			// Create temporary image of what's currently in the buffer
			bool bGotImage = false;
			RImage image;
			if (image.CreateImage(ptxt->m_im.m_sWidth, ptxt->m_im.m_sHeight, ptxt->m_im.m_type) == 0)
				{
				rspLockBuffer();
				rspBlit(g_pimScreenBuf, &image, ptxt->m_sX, ptxt->m_sY, 0, 0, image.m_sWidth, image.m_sHeight);
				bGotImage = true;
				rspUnlockBuffer();
				}

			// Note that the GUI locks the buffer, if appropriate.
			ptxt->Draw(g_pimScreenBuf);
			rspUpdateDisplay(ptxt->m_sX, ptxt->m_sY, ptxt->m_im.m_sWidth, ptxt->m_im.m_sHeight);

			if (bGotImage)
				{
				rspLockBuffer();
				rspBlit(&image, g_pimScreenBuf, 0, 0, ptxt->m_sX, ptxt->m_sY, image.m_sWidth, image.m_sHeight);
				rspUnlockBuffer();
				}
			}

		// Check for the abort key
		if (pau8KeyStatus[NET_PROB_GUI_ABORT_SK_KEY])
			sAbort = 1;
		}

	// If aborting, set our own flag, too
	if (sAbort)
		{
		// Set flag
		ms_bNetBlockingAbort = true;
		}

	return sAbort;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Initialize the net problems GUI.
// Note that this is nearly instantaneous (no file access) if it has already
// been called.  That is, calling this twice in a row is fine.  Just make
// sure you call KillNetProbGUI() when done (although, even that is not
// essential).
//
//////////////////////////////////////////////////////////////////////////////
extern short InitNetProbGUI(void)
	{
	short	sRes	= 0;	// Assume success.

	KillNetProbGUI();

	sRes	= rspGetResource(&g_resmgrShell, NET_PROB_GUI_FILE, &ms_ptxtNetProb);

	if (sRes == 0)
		{
		// Set some intial stuff.
		ms_ptxtNetProb->m_sTextEffects			= RGuiItem::Shadow;
		ms_ptxtNetProb->m_u32TextShadowColor	= NET_PROB_TEXT_SHADOW_COLOR_INDEX;
		ms_ptxtNetProb->m_sX							= NET_PROB_GUI_X;
		ms_ptxtNetProb->m_sY							= NET_PROB_GUI_Y;
		// Set generic initial text.
		ms_ptxtNetProb->SetText("%s", g_pszNetProb_General);
		ms_ptxtNetProb->Compose();
		}
	else
		{
		TRACE("InitNetProbGUI(): Error loading Net Prob GUI \"%s\".\n", 
			NET_PROB_GUI_FILE);
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Kill the net problems GUI.
//
//////////////////////////////////////////////////////////////////////////////
extern void KillNetProbGUI(void)
	{
	if (ms_ptxtNetProb)
		{
		rspReleaseResource(&g_resmgrShell, &ms_ptxtNetProb);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the net prob GUI which is used to notify the user of network problems.
// You can move it, draw it, change its text, or whatever.  It's just up to 
// you to avoid having a problem with other things updating its settings while
// you are.
// Note that this is automagically drawn to the screen via the blocking 
// callback.
//
//////////////////////////////////////////////////////////////////////////////
extern RTxt* GetNetProbGUI(void)
	{
	return ms_ptxtNetProb;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Determine whether there is a net problem.  Set via blocking callback.
// Cleared by you when you call ClearNetProb().
//
//////////////////////////////////////////////////////////////////////////////
extern bool IsNetProb(void)	// Returns true, if net problem; false otherwise.
	{
	return m_bNetWatchdogExpired;
	}

extern void ClearNetProb(void)
	{
	m_bNetWatchdogExpired = false;
	}


///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
