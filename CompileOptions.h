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
// CompileOptions.h
// Special compile options
//
//	History:
//		07/21/97	JMI	Started tracking history of this file.
//							Disabled DISABLE_EDITOR_SAVE_AND_PLAY.
//
//		07/23/97 BRH	Changed expiration date.
//
//		07/23/97	JMI	'Uncommented-out' or 'Commented-in' the 
//							DISABLE_EDITOR_SAVE_AND_PLAY as per Online Beta Demo Patch
//							Marketing Extravaganza Fiasco blah blah blah.
//
//		07/28/97	JMI	Added MARKETING_RELEASE to indicate that it is, well, a
//							marketing release.
//
//		08/17/97	JMI	Commented out CHECK_EXPIRATION_DATE.
//
//		08/18/97	JMI	Added SALES_DEMO macro.
//
//		09/03/97	JMI	Added USE_NEW_CHEATS which signifies to input that it 
//							should use the new cheats.
//
//		09/04/97	JMI	Enabled USE_NEW_CHEATS as we should use it from now on.
//
//		09/11/97	JMI	Modified ENABLE_PLAY_SPECIFIC_REALMS_ONLY to be 
//							"skirts.rlm".
//							Also, added COMP_USA_RELEASE for Bill's changes to game.cpp.
//
//		09/11/97	JMI	Changed ENABLE_PLAY_SPECIFIC_REALMS_ONLY so it was a mere
//							switch and not the realm name that can be played.
//
//		09/11/97	JMI	Added commented out SHAREWARE_RELEASE.
//
//		09/11/97 MJR	Added macros for specific realms.
//
//		09/12/97 MJR	Set macros for the "CompUsa" version
//
//		09/16/97 MJR	Renamed to the more-correct ONLINE_DEMO_RELEASE.
//							Added conditional compilation for various options.
//
//		09/17/97	JMI	Setup correct options for ONLINE_DEMO_RELEASE (correctly
//							known as SHAREWARE_RELEASE :) ).
//							Also, added USE_LA_PALOMA_CHEATS which is like the 'DOS for 
//							Dummies' of cheats, allowing us to keep the 'real' cheats
//							anonymous and make them easy for the editors' demo.
//
//		09/17/97	JMI	Commented out USE_LA_PALOMA_CHEATS.
//
//		09/18/97	JMI	Added EXPIRATION_MSG_POSTAL_LAUNCH_WEEKEND.
//
//		09/18/97	JMI	Put all settings back to the online demo settings and
//							tried to categorize a little.
//
//		09/23/97	JMI	Added localization macros.
//
//		09/24/97	JMI	Commented out ONLINE_DEMO_RELEASE and 
//							ENABLE_PLAY_SPECIFIC_REALMS_ONLY.
//
//		09/25/97	JMI	Commented out EDITOR_DISABLED.
//
//		10/10/97	JMI	Added ALLOW_JOYSTICK macro which enables joystick support.
//
//		10/16/97	JMI	Added NO_MPATH macro which, when defined, causes exclusion
//							of MPath in game.cpp.
//
//		10/23/97	JMI	Uncommented out (i.e., commented in) NO_MPATH macro.
//
//		11/21/97	JMI	Added ADD_ON_PACK macro.
//
//		09/27/99	JMI	Temporarily (perhaps permanently) removed the must-be-on-cd
//							and check-for-cd macros.
//
//					JMI	Added JAPAN and VIOLENT_LOCALE macros.
//
//		10/07/99	JMI	Removed ADD_ON_PACK macro (from Special Delivery).
//							Added comment on setup of JAPAN_ADD_ON and SUPER_POSTAL 
//							macros.
//
//							Changed JAPAN_ADD_ON and SUPER_POSTAL to work more like the
//							LOCALE macro where there's a macro TARGET defined to either
//							JAPAN_ADD_ON or SUPER_POSTAL.  JAPAN_ADD_ON and 
//							SUPER_POSTAL are defined in this file and TARGET is defined
//							in the project settings.
//
//		02/04/00 MJR	Added PROMPT_FOR_ORIGINAL_CD and enabled it in the case of
//							the JAPAN_ADD_ON.
//							Also cleaned up the whole file to make it easier to read.
//
//		03/30/00 MJR	Another huge clean up plus lots of new documentation.
//							Added REQUIRE_POSTAL_CD as a subset of what CHECK_FOR_CD
//							used to do.
//							Added POSTAL_PLUS and added appropriate support for it.
//							Moved APP_NAME and PREFS_FILE macros into here.
//
//							More docs and a few minor tweaks.
//
//							Cleaned up docs a bit more.
//							Added START_MENU_ADDON_ITEM for POSTAL_PLUS.
//
//		06/24/01 MJR	Added DEMO macro to support the creation of demo
//							versions of whatever TARGET is set to.  HOWEVER,
//							I only paid attention to the POSTAL PLUS target, so
//							demo version of other targets may or may not work!
//
//							Added SHOW_EXIT_SCREEN macro (see description below).
//
//							Renamed some of the older macros to make it more clear
//							what they do.  Only did this because they seemed related
//							to the new DEMO macro, although in the end it turned out
//							they were only vaguely related.
//
//							Got rid of the CompUSA macros, which are totally obsolete.
//
//							Added MULTIPLAYER_DISABLED macro (see description below).
//
////////////////////////////////////////////////////////////////////////////////
//
// This file basically determines which game configuration will be built.
//
// There are three key macros that control the compilation of the game:
//		TARGET
//		LOCALE
//		SPAWN
// These macros should be set via the compiler preprocessor options.
// They are described in detail in separate sections below.
//
// In general, any code that uses the TARGET and LOCALE macros should belong
// to a project that defines those macros.  In that case, the code can 
// assume the macros are defined and set to valid values (there are tests
// below to validate the macros).
//
// However, it is possible that a header file that uses TARGET or LOCALE
// might be included by files belonging to a project that does NOT define
// those macros.  In that case, the header should properly handle the case
// where the macros are not defined. (For example: SampleMaster.h)
//
// There are many other macros defined in this file that are dependant
// on TARGET and LOCALE.  This creates an indirect dependency that may not
// be immediately obvious.  It's something to keep in mind when writing
// code that uses any of these macros.
//
// Additional sections below provide details on other macros.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef COMPILE_OPTIONS_H
#define COMPILE_OPTIONS_H


////////////////////////////////////////////////////////////////////////////////
// The TARGET macro must be set equal to one of the macros defined below.
// It should be set in Project Settings (in the C/C++ Preprocessor section).
//
// To add a new target, just define one below using the next available value,
// then make the necessary source file changes to accomodate it.
//
// Do a Find-In-Files for "TARGET" to find out what features are currently
// based on this macro.
//
// Note that there are no targets here for the original Postal or for
// Special Delivery.  This is because this scheme didn't exist back then.
// The source you're working with now is based on the original source, but
// has in fact been branched off entirely.  It is not possible to actually
// rebuild those two titles EXACTLY from this source base.  Many features
// and changes have been made, such as faster multiplayer, faster rendering,
// and a multitude of other, smaller changes.  If you did want to recreate
// the original versions, you'd have to use the original source base.
////////////////////////////////////////////////////////////////////////////////

// This is an add-on pack that adds Japanese voice and a couple of Japanese
// levels to the original Postal.
#define JAPAN_ADD_ON		1

// This is a standalone product that used Japanese voice and included all the
// original levels, Special Delivery levels, and two new Japanese levels.
#define SUPER_POSTAL		2

// This is a standalone product that combines the original levels and
// Special Delivery levels.
#define POSTAL_PLUS		3

// Added a define for POSTAL_2015 (Steam version)
#define POSTAL_2015		4

// If it's defined, make sure it's valid.  See end of file for what
// happens when it's not defined.
#if defined(TARGET)
	#if   TARGET == JAPAN_ADD_ON
	#elif TARGET == SUPER_POSTAL
	#elif TARGET == POSTAL_PLUS
	#elif TARGET == POSTAL_2015
	#else 
		#error TARGET is set to an unrecognized value
	#endif
#endif


////////////////////////////////////////////////////////////////////////////////
// The LOCALE macro must be set equal to one of the macros defined below.
// It should be set in Project Settings (in the C/C++ Preprocessor section).
//
// To add a new locale, just define one below using the next available value,
// then make the necessary source file changes to accomodate it.
//
// Do a Find-In-Files for "LOCALE" to find out what features are currently
// based on this macro.
////////////////////////////////////////////////////////////////////////////////

#define US					1
#define UK					2
#define ARIAN				3
#define MIME				4
#define JAPAN				5

// Synomyms for the humor-impaired
#define GERMAN				ARIAN
#define FRENCH				MIME

// If it's defined, make sure it's valid.  See end of file for what
// happens when it's not defined.
#if defined(LOCALE)
	// Verify that it was set to a good value
	#if   LOCALE == US
	#elif LOCALE == UK
	#elif LOCALE == ARIAN
	#elif LOCALE == MIME
	#elif LOCALE == JAPAN
	#else 
		#error LOCALE is set to an unrecognized value
	#endif
#endif


// This macro is true when the LOCALE allows the full level of violence
#define VIOLENT_LOCALE	(LOCALE == US || LOCALE == JAPAN)


// This macro is true when the LOCALE is okay with the use of English text
#define ENGLISH_LOCALE	(LOCALE == US || LOCALE == UK || LOCALE == JAPAN)


////////////////////////////////////////////////////////////////////////////////
// The DEMO macro should be defined to create a demo version.
// It should be set in Project Settings (in the C/C++ Preprocessor section).
// 
// A demo version is based on the full product but includes only one level
// and does not include the editor or multiplayer support.
//
// DEMO is currently only setup to work with the TARGET set to POSTAL PLUS.
// It's relatively easy to get it to work with other targets, but since
// they weren't needed at the time, and I didn't want to test them, I
// didn't bother adding support for that.
//
// To create a DEMO version, all that is necessary is to define the macro.
// If you don't define it, you get the normal version of the game.
//
// You can NOT use DEMO and SPAWN at the same time!
////////////////////////////////////////////////////////////////////////////////

#if defined (SPAWN) && defined (DEMO)
	#error DEMO and SPAWN cannot be used together.
#endif


////////////////////////////////////////////////////////////////////////////////
// The SPAWN macro should be defined to create a multiplayer-only version.
// It should be set in Project Settings (in the C/C++ Preprocessor section).
// 
// "Spawn" is (or was) the popular term for a limited, multiplayer version of
// a game that was included on the game CD so that other people could play
// without having to buy their own copy of the game.
//
// To create a SPAWN version, all that is necessary is to define the macro.
// If you don't define it, you get the normal version of the game.
//
// You can NOT use DEMO and SPAWN at the same time!
////////////////////////////////////////////////////////////////////////////////

#if defined (SPAWN) && defined (DEMO)
	#error DEMO and SPAWN cannot be used together.
#endif


////////////////////////////////////////////////////////////////////////////////
// Set APP_NAME to the text to be displayed in various message boxes and used
// as the title of the main application window.
//
// Set MAIN_MENU_TITLE to the title for the main menu.  This should generally
// match APP_NAME, except it should be all-caps.
//
// In previous products, these names were always "POSTAL" which seems stupid in
// retrospect.  Now, the idea is to set them to match the product's consumer
// name (which is probably different than its internal project name).
//
//		Note that these macros do NOT effect the executable name!  Use Project
//		Settings to verify the executable name.  By default, it seems to be
//		based on the Project (.dsp) name, but if it was ever modified manually
//		then it may need to be updated manually.  And again, the executable
//		name should be set to the product's consumer name.
//
////////////////////////////////////////////////////////////////////////////////

#if defined(TARGET) && defined(LOCALE)

	#if TARGET == POSTAL_PLUS
		#if defined(SPAWN)
			#define APP_NAME				"Postal Plus MP"
			#define MAIN_MENU_TITLE		"POSTAL PLUS MP"
		#elif defined (DEMO)
			#define APP_NAME				"Postal Plus Demo"
			#define MAIN_MENU_TITLE		"POSTAL PLUS DEMO"
		#else
			#define APP_NAME				"Postal Plus"
			#define MAIN_MENU_TITLE		"POSTAL PLUS"
		#endif
	#else
		// Prior to this point, all targets used the same name, so for
		// consistency sake, let's leave it that way.
		#define APP_NAME				"Postal"
		#define MAIN_MENU_TITLE		"POSTAL"
	#endif

#endif


////////////////////////////////////////////////////////////////////////////////
// Preference file name (in other words, the all important .ini file).
////////////////////////////////////////////////////////////////////////////////

#if defined(TARGET) && defined(LOCALE)

	#if LOCALE == US
		#if TARGET == POSTAL_PLUS
			#if defined(SPAWN)
				#define PREFS_FILE	"Postal Plus MP.ini"
			#elif defined (DEMO)
				#define PREFS_FILE	"Postal Plus Demo.ini"
			#else
				#if PLATFORM_UNIX
					#define PREFS_FILE	"postal_plus.ini"
				#else
					#define PREFS_FILE	"Postal Plus.ini"
				#endif
			#endif
		#else
			#define PREFS_FILE	"POSTAL.INI"
		#endif
	#elif LOCALE == UK
		#define PREFS_FILE	"PostalUK.INI"
	#elif LOCALE == FRENCH
		#define PREFS_FILE	"PostalFr.INI"
	#elif LOCALE == GERMAN
		#define PREFS_FILE	"PostalGr.INI"
	#elif LOCALE == JAPAN
		#if TARGET == JAPAN_ADD_ON
			#define PREFS_FILE	"Postal Japan Add On.ini"
		#elif TARGET == SUPER_POSTAL
			#define PREFS_FILE	"Super Postal.ini"
		#endif
	#endif

#endif


////////////////////////////////////////////////////////////////////////////////
// The following macros can be defined in order to enable/disable various
// features.
//
//		EDITOR_DISABLED			When defined, the editor item on the main menu is
//										disabled, and much of the editor code is not
//										included in the executable.
//
//		REQUIRE_POSTAL_CD			When defined, the program will check to see if
//										the CD appears to be the original Postal CD.  It
//										does this by checking that one particular file
//										exists and one other particular file doesn't
//										exist.  This was designed to tell the difference
//										between the Postal and Special Delivery CD's.
//										Now that several other Postal-related CD's have
//										been released, this particular test will not
//										be able to properly tell them apart.  Therefore,
//										this macro is pretty much obsolete.
//
//										Idea: If the need to differentiate CD's comes up
//										again, we might be able to use the fact that
//										newer versions (Japan and Postal Plus) have used
//										specifically-named ini files.
// 
//		MUST_BE_ON_CD				When defined, the program makes sure that the
//										drive refered to by the CD path is actually a
//										CD-ROM drive as far as the operating system knows.
//										This only works for Windows.
//		
//		CHECK_FOR_COOKIE			When defined, the program will check for a
//										"cookie" (special value) at a particular position
//										in a particularly large file on the CD.  If the
//										cookie is incorrect, the program will not operate
//										correctly.  This would typically be used when you
//										want an extra degree of anti-piracy.  This was
//										used by the original Postal.
//
//		DISABLE_EDITOR_SAVE_AND_PLAY	When defined, the editor's save and play
//										features are both disabled.  This would typically
//										be used for single-level demo releases, where you
//										don't want players to be able to create their own
//										levels because they haven't yet bought the game.
//
//		PROMPT_FOR_ORIGINAL_CD	When defined, the player is prompted (before the
//										title screens even appear) to make sure the
//										original CD is inserted.  No actual test is done
//										to verify that the CD was inserted.  This would
//										typically be used for "add-on packs" that require
//										the original CD.
//
//		START_MENU_ADDON_ITEM	When defined, an additional item appears on the
//										start menu that allows the user to start on the
//										first add-on level.  If this macro is used, you
//										must add customized text for the menu items
//										in localize.cpp.
//
//		TITLE_SHOW_DISTRIBUTOR	When defined, the distributor screen is shown.
//										Otherwise, it is not.
//
//		SHOW_EXIT_SCREEN			When defined, we show a screen when the player
//										chooses to exit the app.  The screen stays up
//										until the player clicks or hits a button.
//
//		MULTIPLAYER_DISABLED		When defined, all multiplayer support is disabled.
//
//		ENABLE_PLAY_SPECIFIC_REALMS_ONLY		When defined, the game will only
//										play specific realms.  See elsewhere for more
//										info on how this works.
//
// The above macros should be defined below (as appropriate) based on the
// current TARGET, LOCALE and SPAWN macros.
////////////////////////////////////////////////////////////////////////////////

#if defined(TARGET) && defined(LOCALE)

	#if TARGET == JAPAN_ADD_ON
		#if defined(SPAWN)
			#define EDITOR_DISABLED
		#else
			#define PROMPT_FOR_ORIGINAL_CD
			#define START_MENU_ADDON_ITEM
			#define TITLE_SHOW_DISTRIBUTOR
		#endif
	#elif TARGET == SUPER_POSTAL
		#if defined(SPAWN)
			#define EDITOR_DISABLED
		#else
			#define TITLE_SHOW_DISTRIBUTOR
		#endif
	#elif TARGET == POSTAL_PLUS
		#if defined(SPAWN)
			#define EDITOR_DISABLED
		#elif defined (DEMO)
			#define EDITOR_DISABLED
			#define MULTIPLAYER_DISABLED
			#define ENABLE_PLAY_SPECIFIC_REALMS_ONLY
			#define SHOW_EXIT_SCREEN
		#elif defined(WIN32)
			//#define MUST_BE_ON_CD
			//#define START_MENU_ADDON_ITEM
		#endif
	#elif TARGET == POSTAL_2015
		#define MULTIPLAYER_REMOVED
		#define EDITOR_REMOVED
		#define LOADLEVEL_REMOVED
		#define START_MENU_ADDON_ITEM
	#endif
#endif

// Turn off multiplayer on MacOSX/Linux...not worth it.  --ryan.
//  This just takes it out of the menus...code is still compiled in.
#if 1 //PLATFORM_UNIX
    #define MULTIPLAYER_REMOVED
	#define EDITOR_REMOVED
    #define LOADLEVEL_REMOVED  // bleh, no file dialog thingey.  :/
#endif


////////////////////////////////////////////////////////////////////////////////
// Define the character to be used as the separator in the audio sak filenames.
//
// This was originally hard-wired to "_" which gave us names like
//		11025_16.sak
// where the separator is used between the rate and the number of bits.
//
// For the Japanese version of Postal, the audio was re-recorded in Japanese.
// If this had been purely a stand-alone product, no change would have been
// required in the audio sak filenames.  However, it was also available as
// an add-on that would change all the audio in the original Postal to the new
// Japanese audio.  However, we also wanted the user to be able to go back to
// the original Postal installation and play the game with the original English
// audio.  Therefore, a new naming scheme was devised so we could keep both
// versions of the audio files in the same folder.  For the Japanese versions,
// the "_" was changed to "j".
//
// That led to the creation of this macro to make this feature easy to maintain.
////////////////////////////////////////////////////////////////////////////////

#if defined(TARGET) && defined(LOCALE)

	#if LOCALE == JAPAN
		#define AUDIO_SAK_SEPARATOR_CHAR		'j'
	#else
		#define AUDIO_SAK_SEPARATOR_CHAR		'_'
	#endif

#endif


////////////////////////////////////////////////////////////////////////////////
// Cheating
//
// These macros control which cheat-keys are enabled.  If USE_LA_PALOMA_CHEATS
// is defined, then USE_NEW_CHEATS has no effect.  Since the LaPaloma cheats
// were designed just to be used for a specific event, they should probably
// never be needed again.  And since the "new cheats" were designed to replace
// values that had become public too soon, that should probably always be
// defined, unless we decide to change them again some time in the future.
////////////////////////////////////////////////////////////////////////////////

//#define USE_LA_PALOMA_CHEATS
#define USE_NEW_CHEATS

// Enable this for sales demos.  This enables a cheat ("sell") that gives you
// everything plus invincibility and ability to go to next level at any time.
//#define SALES_DEMO


////////////////////////////////////////////////////////////////////////////////
// Expiration Check
//
// When CHECK_EXPIRATION_DATE is defined, the game will check to see whether
// it has expired (according to the dates defined below) and if so, it will
// no longer play properly (all the characters will burst into flames shortly
// after the start of a level).
//
// If this feature is used, then you need to set the release and expiration
// date values.
//
// RELEASE_DATE is the earliest date the game will be run.  If it is run
// before this date, it will assume the user is trying to circumvent the
// expiration feature, and it will consider itself expired.
//
// EXPIRATION_DATE is the latest date the game will run.  After this date,
// it will be expired.
//
// SAFE_DATE appears to be some date far in the future.  I can't remember
// what it was used for.
//
// Dates are calculated using the following math:
//
//		Time to 1/1/97 from 1/1/70 = 852076800
//		1/1/70 = 0
//		1/1/97 = (365*27) + 7 extra leap year days) * 24 * 60 * 60 = 852076800
//		7/17/97 = 198th day in the year = 198 * 24 * 60 * 60
//		9/22/97 = 265th day in the year = 265 * 24 * 60 * 60 = 22896000
//
////////////////////////////////////////////////////////////////////////////////

#define RELEASE_DATE						869443200	// Real release date - change before sending 7/20/97
#define EXPIRATION_DATE					(22896000+852076800-1)	// 11:59pm Sepember 30, 1997 (day 273)
#define SAFE_DATE							2047483000	// We'll all be dead.

// These macros cause customized expiration messages to be displayed instead
// of the standard expiration message.  These particular messages are obsolete,
// but I left these in just to illustrate how the messages could be customized.
//#define EXPIRATION_MSG_MARKETING_RELEASE
//#define EXPIRATION_MSG_POSTAL_LAUNCH_WEEKEND


////////////////////////////////////////////////////////////////////////////////
// Level Limitations
//
// The game can be set to only allow playing of a specific realm.  This is
// normally used for demos which are limited to one level.
//
// To enable this feature, simply define ENABLE_PLAY_SPECIFIC_REALMS_ONLY.
//
// The following macros determine which realm will be used for multiplayer
// games.  Note that the text, file name, and number MUST match the associated
// values in the "standard" realms.ini!!!  Also note that changing these values
// to a different realm is only one step -- you must also change the files
// being included by MemFileFest.cpp!!!
////////////////////////////////////////////////////////////////////////////////

#if defined(ENABLE_PLAY_SPECIFIC_REALMS_ONLY)
	#define SPECIFIC_MP_REALM_TEXT			"Build Your Own Death"
	#define SPECIFIC_MP_REALM_FILE			"res/levels/multi/mpconsit.rlm"
	#define SPECIFIC_MP_REALM_NUM				10
#endif


////////////////////////////////////////////////////////////////////////////////
// Miscellaneous Stuff
////////////////////////////////////////////////////////////////////////////////

#define ALLOW_JOYSTICK

#endif // COMPILE_OPTIONS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
