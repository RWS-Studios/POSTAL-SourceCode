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
// menus.h
// Project: Nostril (aka Postal)
//
// History:
//		12/05/96 MJR	Started.
//
//		04/18/97	JMI	Added extern for menuEditor.
//
//		07/05/97	JMI	Moved menu IDs and menu externs into here.
//
//		07/13/97	JMI	Added declaration of menuChallenge.
//
//		07/16/97	JMI	Added menuVolumes and path from menuOptions.
//
//		07/20/97	JMI	Added menuVideoOptions, menuAudioOptions, and 
//							menuPlayOptions.
//
//		08/04/97	JMI	Added menuRotation.
//
//		08/15/97	JRD	Added separate postal organ menu,
//
//		08/20/97 MJR	Added separate multi-join and multi-host menus.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MENUS_H
#define MENUS_H


#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/Menu/menu.h"
#else
	#include "menu.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Macros.
////////////////////////////////////////////////////////////////////////////////

#define MAIN_MENU_ID						1
#define VERIFY_EXIT_MENU_ID			2
#define CLIENT_GAME_MENU_ID			3
#define HIGH_SCORES_MENU_ID			4
#define OPTIONS_MENU_ID					5
#define START_MENU_ID					6
#define VERIFY_QUIT_GAME_MENU_ID		7
#define MULTIPLAYER_OPTIONS_MENU_ID	8
#define EDITOR_MENU_ID					9
#define KEYBOARD_MENU_ID				10
#define MOUSE_MENU_ID					11
#define JOYSTICK_MENU_ID				12
#define CONTROLS_MENU_ID				13
#define START_SINGLE_MENU_ID			14
#define START_MULTI_MENU_ID			15
#define START_DEMO_MENU_ID				16
#define FEATURES_MENU_ID				17
#define CHALLENGE_MENU_ID				18
#define VOLUME_MENU_ID					19
#define VIDEO_MENU_ID					20
#define AUDIO_MENU_ID					21		
#define PLAYOPTIONS_MENU_ID			22
#define ROTATION_MENU_ID				23
#define ORGAN_MENU_ID					24
#define JOIN_MULTI_MENU_ID				25
#define HOST_MULTI_MENU_ID				26
#define PICK_FILE_MENU_ID				27

////////////////////////////////////////////////////////////////////////////////
// Externs.
////////////////////////////////////////////////////////////////////////////////

// Forward declarations.
extern Menu	menuStart;
extern Menu menuStartSingle;
extern Menu menuStartMulti;
extern Menu menuStartDemo;
extern Menu	menuOptions;
extern Menu menuControls;
extern Menu menuKeyboard;
extern Menu menuMouse;
extern Menu menuJoystick;
extern Menu menuHighScores;
extern Menu	menuVerifyExit;
extern Menu menuMultiOptions;
extern Menu	menuFeatures;
extern Menu menuChallenge;
extern Menu menuVolumes;
extern Menu	menuVideoOptions;
extern Menu	menuAudioOptions;
extern Menu	menuPlayOptions;
extern Menu menuRotation;
extern Menu menuJoinMulti;
extern Menu menuHostMulti;

// Main menu
extern Menu	menuMain;

// Client-game menu
extern Menu menuClientGame;

// Editor menu.
extern Menu menuEditor;

// Verify Quit Game.
extern Menu	g_menuVerifyQuitGame;

#if 1 //PLATFORM_UNIX

#ifdef MOBILE
#define MAX_SAVE_SLOTS 5
#else
#define MAX_SAVE_SLOTS 9
#endif

short PickFile(const char *title, void (*enumer)(Menu *), char *buf, size_t bufsize);
#endif

#endif //MENUS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
