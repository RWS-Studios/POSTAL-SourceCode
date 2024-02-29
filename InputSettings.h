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
// InputSettings.h
// Project: Nostril (aka Postal)
//
// History:
//
//		03/31/97	JMI	Started.
//
//		04/25/97	JMI	Added execute input.
//
//		05/13/97	JMI	Changed 'Run' to 'Walk'.
//							Also, added 'Suicide' and 'NextLevel'.
//
//		05/14/97	JMI	Added pick up.
//
//		06/09/97	JMI	Changed 'Walk' to 'Run'.
//
//		06/10/97	JMI	Changed m_dDegreesPerSec to m_dMovingSlowDegreesPerSec and
//							added m_dMovingFastDegreesPerSec, m_dStillSlowDegreesPerSec,
//							m_dStillFastDegreesPerSec, and m_sTapRotationDegrees.
//
//		06/15/97 MJR	Removed NextLevel.
//
//		07/03/97	JMI	Moved InputInfo and ms_ainputinfo into CInputSettings.
//
//		07/06/97	JMI	Changed m_au8PlayKeys[] from a U8 array to a short array,
//							m_asPlayKeys[].
//							Also, changed m_asPlayButtons to m_asPlayMouseButtons.
//
//		07/07/97	JMI	Added m_dMouseSensitivityX and m_dMouseSensitivityY.
//
//		07/16/97	JMI	Added Weapon10 enum.
//
//		07/25/97	JMI	Removed PickUp enum.
//
//		08/04/97	JMI	Added DefaultRotations().
//
//		08/10/97	JMI	Added StrafeLeft and StrafeRight inputs.
//							Also, changed Jump to Revive.
//
//		08/12/97	JMI	Removed StrafeLeft and StrafeRight inputs.  Replaced with
//							Strafe2 which can get you the same thing if you combine 
//							both strafes with an turn arrow key.
//							Also, added Run2 which is handy for similar reasons.
//
//		08/24/97	JMI	Moved Execute and Suicide to before the weapons and changed
//							the weapon descriptions to reflect the actual weapons.
//
//		08/27/97	JMI	Added pszSaveName to InputInfo.
//
//		08/27/97	JMI	Added Fire2.
//
//		10/10/97	JMI	Added m_sUseJoystick, JoyButtons, InputInfo.sJoyButtons.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INPUTSETTINGS_H
#define INPUTSETTINGS_H

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/Prefs/prefs.h"
#else
	#include "prefs.h"
#endif

#include "settings.h"
#include "localize.h"

// Game settings
class CInputSettings : CSettings
	{
	///////////////////////////////////////////////////////////////////////////
	// Typedefs, enums.
	///////////////////////////////////////////////////////////////////////////
	public:
		typedef enum
			{
			Left,
			Right,
			Forward,
			Backward,
			Up,
			Down,
			MoveLeft,
			MoveRight,
			StrafeLeft,
			StrafeRight,
			Run,
			Run2,
			Strafe,
			Strafe2,
			Fire,
			Fire2,
			FireLeft,
			FireRight,
			FireUp,
			FireDown,
			Duck,
			Revive,
			Execute,
			Suicide,
			NextMap,
			NextWeapon,
			PrevWeapon,
			Weapon0,
			Weapon1,
			Weapon2,
			Weapon3,
			Weapon4,
			Weapon5,
			Weapon6,
			Weapon7,
			Weapon8,
			Weapon9,
			Weapon10,

			NumInputFunctions
			} InputFunction;

		typedef enum
			{
			None				= 0,
			LeftButton		= 1,
			RightButton		= 2,
			MiddleButton	= 4
			} MouseButtons;

		typedef enum
			{
//			None				= 0,
			ButtonA			= 1,
			ButtonB			= 2,
			ButtonC			= 4,
			ButtonD			= 8
			} JoyButtons;

		// Game play input information.
		typedef struct
			{
			char*	pszDescription;	// Description of key.
			char*	pszSaveName;		// Name for INI.
			U8		u8DefaultKey;		// Default rspScanKeys val (RSP_SK_*).
			short	sDefMouseButtons;	// Default rspGetMouse psButtons mask (MouseButtons).
			short	sDefJoyButtons;	// Default rspGetJoyState buttons mask (JoyButtons).
			} InputInfo;


	///////////////////////////////////////////////////////////////////////////
	// Static members.
	///////////////////////////////////////////////////////////////////////////
	public:

		// Function descriptions and default input settings.
		static InputInfo	ms_ainputinfo[CInputSettings::NumInputFunctions];
		static InputInfo	ms_ainputinfoOld[CInputSettings::NumInputFunctions];

	///////////////////////////////////////////////////////////////////////////
	// Instantiable members.
	///////////////////////////////////////////////////////////////////////////
	public:
		double	m_dMovingSlowDegreesPerSec;	// Degrees of rotation per second that the
															// user can achieve with the left and right
															// arrow keys when moving without the fast
															// key held down (for main dude input).
		double	m_dMovingFastDegreesPerSec;	// Degrees of rotation per second that the
															// user can achieve with the left and right
															// arrow keys when moving while holding the 
															// 'fast' key (for main dude input).
		double	m_dStillSlowDegreesPerSec;		// Degrees of rotation per second that the
															// user can achieve with the left and right
															// arrow keys when moving without the fast
															// key held down (for main dude input).
		double	m_dStillFastDegreesPerSec;		// Degrees of rotation per second that the
															// user can achieve with the left and right
															// arrow keys when moving while holding the 
															// 'fast' key (for main dude input).
		double	m_sTapRotationDegrees;			// Rotation when 'tapping' a rotate key.

		double	m_dMouseSensitivityX;			// Percentage of X mouse movement utilized.
		double	m_dMouseSensitivityY;			// Percentage of Y mouse movement utilized.
															// For example, if 0.50, mouse movement is
															// halved before interpretation by the
															// input logic.

		short		m_sUseMouse;						// Allow mouse input (for main dude).

		short		m_sUseJoy;							// Allow joystick input.

		U32	m_asPlayKeys[NumInputFunctions];				// Array of game play keys indexed
																		// by an InputFunction value.
		U32	m_asPlayMouseButtons[NumInputFunctions];	// Array of game play mouse buttons
																		// indexed by an InputFunction value.
		U32	m_asPlayJoyButtons[NumInputFunctions];		// Array of game play joystick buttons
																		// indexed by an InputFunction value.
		short	m_sJoyStartButton;						// Default button to use as "Start"
		short	m_sJoyMenuUpButton;						// Default button to use as "Menu Up"
		short	m_sJoyMenuDownButton;						// Default button to use as "Menu Down"
		short	m_sJoyMenuLeftButton;						// Default button to use as "Menu Left"
		short	m_sJoyMenuRightButton;						// Default button to use as "Menu Right"
		short	m_sJoyMenuUpAxis;						// Default button to use as "Menu Up"
		short	m_sJoyMenuDownAxis;						// Default button to use as "Menu Down"
		short	m_sJoyMenuLeftAxis;						// Default button to use as "Menu Left"
		short	m_sJoyMenuRightAxis;						// Default button to use as "Menu Right"
		short	m_sJoyMenuConfirmButton;						// Default button to use as "Confirm"
		short	m_sJoyMenuBackButton;						// Default button to use as "Back"
		short	m_sJoyMenuBackButton2;						// Other default button to use as "Back"
		short	m_sJoyMenuDeleteKeybindButton;			// Default button to use for deleting keybinds

	public:
		// Set settings to default values
		CInputSettings(void);

		// Destructor
		~CInputSettings();

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

		// Set rotation values to the defaults.
		void DefaultRotations(void);

	};

#endif // INPUTSETTINGS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
