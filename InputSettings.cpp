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
// InputSettings.cpp
// Project: Nostril (aka Postal)
//
// History:
//		03/31/97 JMI	Started.
//
//		04/25/97	JMI	Added execute input.
//
//		05/13/97	JMI	Changed 'Run' to 'Walk'.
//							Also, added 'Suicide' and 'NextLevel'.
//
//		05/14/97	JMI	Changed key description of Next Level to "NextLevel" b/c
//							prefs does not seem to handle spaces in the var name
//							correctly.
//
//		05/14/97	JMI	Added pick up.
//
//		06/09/97	JMI	Swapped default keys for Strafe and Fire.
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
//							Also, changed g_apszButtonDescriptions to 
//							g_apszMouseButtonDescriptions.
//							Also, changed m_asPlayButtons to m_asPlayMouseButtons.
//
//		07/07/97	JMI	Added m_dMouseSensitivityX and m_dMouseSensitivityY.
//
//		07/16/97	JMI	Added Weapon10 enum.
//
//		07/25/97	JMI	Removed PickUp enum and associated array entries.
//
//		08/04/97	JMI	Added DefaultRotations().
//							Also, rotations (except for tap) were not saving with the
//							same var names they were loading with.  Fixed.
//
//		08/10/97	JMI	Changed defaults for mouse buttons to typical Doom mouse 
//							player style.
//							Added StrafeLeft and StrafeRight inputs.
//							Also, changed Jump to Revive.
//
//		08/12/97	JMI	Removed StrafeLeft and StrafeRight inputs.  Replaced with
//							Strafe2 which can get you the same thing if you combine 
//							both strafes with an turn arrow key.
//							Also, added Run2 which is handy for similar reasons.
//
//		08/24/97	JMI	Moved Execute and Suicide to before the weapons and changed
//							the weapon descriptions to reflect the actual weapons.
//							Also, changed 'No Weapon' to be weapon 0 and 'Mines' to be
//							weapon 10.
//							Also, changed names for Strafe to Strafe1 and Run to Run1.
//
//		08/27/97	JMI	Changed Run1 to Run, Run2 to Run_, Strafe1 to Strafe, and
//							Strafe2 to Strafe_.  The reason I don't want to set them
//							to the same value is b/c these are the values used to 
//							write them to the INI and have to be different.  The deal
//							with using the _ is that it doesn't show up in Smash.fnt
//							so it looks fine.  Cheezy, I know, but quick 
//							implementation!
//
//		08/27/97	JMI	Changed Run_ to Run. and Strafe_ to Strafe. .
//
//		08/27/97	JMI	Added pszSaveName to InputInfo.  Now saves using 
//							pszSaveName.
//
//		08/27/97	JMI	Added Fire2.
//
//		10/10/97	JMI	Added m_sUseJoystick, JoyButtons, InputInfo.sJoyButtons.
//
//////////////////////////////////////////////////////////////////////////////
//
// Implementation for CInputSettings object.  Each instance contains settings
// for Postal input.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////
#include "RSPiX.h"

///////////////////////////////////////////////////////////////////////////////
// Postal Headers.
///////////////////////////////////////////////////////////////////////////////

#include "InputSettings.h"
#include "game.h"
#include "keys.h"

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////
CInputSettings::InputInfo	CInputSettings::ms_ainputinfoOld[CInputSettings::NumInputFunctions]	=
	{	// Description			Save Name			Default Key			Default Mouse Button(s)			Default Joy Button(s)
		//	===========			===========			===========			=======================			=====================
		{	"Turn Left",		"Left",				RSP_SK_A,			0,										0,								},
		{	"Turn Right",		"Right",			RSP_SK_D,			0,										0,								},
		{	"Forward",			"Forward",			RSP_SK_W,			RightButton,							0,								},
		{	"Backward",			"Backward",			RSP_SK_S,			0,										0,								},
		{	"Up",				"Up",				0,					0,										0,	},
		{	"Down",				"Down",				0,					0,										0,	},
		{	"Left",				"MoveLeft",			0,					0,										0,	},
		{	"Right",			"MoveRight",		0,					0,										0,	},
		{	"Strafe Left",		"StrafeLeft",		0,					0,										0,	},
		{	"Strafe Right",		"StrafeRight",		0,					0,										0,	},
		{	"Walk",				"Run1",				RSP_SK_SHIFT,		0,										0,						},
		{	"Walk",				"Run2",				0,					0,										0,								},
		{	"Strafe",			"Strafe1",			RSP_SK_ALT,			MiddleButton,							0,						},
		{	"Strafe",			"Strafe2",			0,					0,										0,								},
		{	"Fire",				"Fire",				RSP_SK_CONTROL,		LeftButton,								0,						},
		{	"Fire",				"Fire2",			0,					0,										0,								},
		{	"Fire Left",		"FireLeft",			0,					0,										0,								},
		{	"Fire Right",		"FireRight",		0,					0,										0,								},
		{	"Fire Up",			"FireUp",			0,					0,										0,								},
		{	"Fire Down",		"FireDown",			0,					0,										0,								},
		{	"Duck",				"Duck",				RSP_SK_F,			0,										RSP_JOY_BUT_1,						},
		{	"Rejuvenate",		"Rejuvenate",		RSP_SK_SPACE,		0,										RSP_JOY_BUT_1,								},
		{	"Execute",			"Execute",			RSP_SK_X,			0,										RSP_JOY_BUT_3,								},
		{	"Suicide",			"Suicide",			RSP_SK_K,			0,										RSP_JOY_BUT_4,								},
		{	"Next Level",		"NextLevel",		RSP_SK_F1,			0,										RSP_JOY_BUT_2,								},
		{	"Next Weapon",		"NextWeapon",		RSP_SK_RBRACKET,	0,										RSP_JOY_BUT_11,								},
		{	"Prev Weapon",		"PrevWeapon",		RSP_SK_LBRACKET,	0,										RSP_JOY_BUT_10,								},
		{	"NoWeapon",			"NoWeapon",			RSP_SK_LQUOTE,		0,										0,								},
		{	"MachineGun",		"MachineGun",		RSP_SK_1,			0,										0,								},
		{	"Shotgun",			"Shotgun",			RSP_SK_2,			0,										0,								},
		{	"SprayCannon",		"SprayCannon",		RSP_SK_3,			0,										0,								},
		{	"Grenades",			"Grenades",			RSP_SK_4,			0,										0,								},
		{	"Missiles",			"Missiles",			RSP_SK_5,			0,										0,								},
		{	"Heatseekers",		"Heatseekers",		RSP_SK_6,			0,										0,								},
		{	"Molotovs",			"Molotovs",			RSP_SK_7,			0,										0,								},
		{	"Napalm",			"Napalm",			RSP_SK_8,			0,										0,								},
		{	"Flamer",			"Flamer",			RSP_SK_9,			0,										0,								},
		{	"Mines",			"Mines",			RSP_SK_0,			0,										0,								},
	};


CInputSettings::InputInfo	CInputSettings::ms_ainputinfo[CInputSettings::NumInputFunctions]	=
	{	// Description			Save Name			Default Key			Default Mouse Button(s)			Default Joy Button(s)
		//	===========			===========			===========			=======================			=====================
		{	"Turn Left",		"Left",				0,					0,										0,								},
		{	"Turn Right",		"Right",			0,					0,										0,								},
		{	"Forward",			"Forward",			0,					0,										0,								},
		{	"Backward",			"Backward",			0,					0,										0,								},
		{	"Up",				"Up",				RSP_SK_W,			0,										0,	},
		{	"Down",				"Down",				RSP_SK_S,			0,										0,	},
		{	"Left",				"MoveLeft",			RSP_SK_A,			0,										0,	},
		{	"Right",			"MoveRight",		RSP_SK_D,			0,										0,	},
		{	"Strafe Left",		"StrafeLeft",		0,					0,										0,	},
		{	"Strafe Right",		"StrafeRight",		0,					0,										0,	},
		{	"Walk",				"Run1",				RSP_SK_SHIFT,		0,										0,						},
		{	"Walk",				"Run2",				0,					0,										0,								},
		{	"Strafe",			"Strafe1",			0,					0,										0,						},
		{	"Strafe",			"Strafe2",			0,					0,										0,								},
		{	"Fire",				"Fire",				RSP_SK_CONTROL,		0,										RSP_JOY_BUT_17,						},
		{	"Fire",				"Fire2",			0,					0,										0,								},
		{	"Fire Left",		"FireLeft",			RSP_SK_LEFT,		0,										0,								},
		{	"Fire Right",		"FireRight",		RSP_SK_RIGHT,		0,										0,								},
		{	"Fire Up",			"FireUp",			RSP_SK_UP,			0,										0,								},
		{	"Fire Down",		"FireDown",			RSP_SK_DOWN,		0,										0,								},
		{	"Duck",				"Duck",				RSP_SK_F,			0,										RSP_JOY_BUT_16,						},
		{	"Rejuvenate",		"Rejuvenate",		RSP_SK_SPACE,		0,										RSP_JOY_BUT_1,								},
		{	"Execute",			"Execute",			RSP_SK_X,			0,										RSP_JOY_BUT_3,								},
		{	"Suicide",			"Suicide",			RSP_SK_K,			0,										RSP_JOY_BUT_4,								},
		{	"Next Level",		"NextLevel",		RSP_SK_F1,			0,										RSP_JOY_BUT_2,								},
		{	"Next Weapon",		"NextWeapon",		RSP_SK_RBRACKET,	0,										RSP_JOY_BUT_11,								},
		{	"Prev Weapon",		"PrevWeapon",		RSP_SK_LBRACKET,	0,										RSP_JOY_BUT_10,								},
		{	"NoWeapon",			"NoWeapon",			RSP_SK_LQUOTE,		0,										0,								},
		{	"MachineGun",		"MachineGun",		RSP_SK_1,			0,										0,								},
		{	"Shotgun",			"Shotgun",			RSP_SK_2,			0,										0,								},
		{	"SprayCannon",		"SprayCannon",		RSP_SK_3,			0,										0,								},
		{	"Grenades",			"Grenades",			RSP_SK_4,			0,										0,								},
		{	"Missiles",			"Missiles",			RSP_SK_5,			0,										0,								},
		{	"Heatseekers",		"Heatseekers",		RSP_SK_6,			0,										0,								},
		{	"Molotovs",			"Molotovs",			RSP_SK_7,			0,										0,								},
		{	"Napalm",			"Napalm",			RSP_SK_8,			0,										0,								},
		{	"Flamer",			"Flamer",			RSP_SK_9,			0,										0,								},
		{	"Mines",			"Mines",			RSP_SK_0,			0,										0,								},
	};

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Set settings to default values
//////////////////////////////////////////////////////////////////////////////
CInputSettings::CInputSettings(void)
	{
	// There is a separate function for these so outside sources can default
	// them at anytime.
	DefaultRotations();

	m_sUseMouse							= FALSE;
	m_sUseJoy							= FALSE;

	m_dMouseSensitivityX				= 1.00;	// 100%
	m_dMouseSensitivityY				= 1.00;	// 100%

	// Set default inputs (in case no INI is loaded).
	short i;
	for (i = 0; i < NumInputFunctions; i++)
		{
		m_asPlayKeys[i]			= ms_ainputinfo[i].u8DefaultKey;
		m_asPlayMouseButtons[i]	= ms_ainputinfo[i].sDefMouseButtons;
		m_asPlayJoyButtons[i]	= ms_ainputinfo[i].sDefJoyButtons;
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
CInputSettings::~CInputSettings()
	{
	}

//////////////////////////////////////////////////////////////////////////////
// Read settings that are stored in preference file
//////////////////////////////////////////////////////////////////////////////
short CInputSettings::LoadPrefs(
	RPrefs* pPrefs)
	{
	short sResult = 0;

	pPrefs->GetVal("Input", "WalkTurnRate", m_dMovingSlowDegreesPerSec, &m_dMovingSlowDegreesPerSec);
	pPrefs->GetVal("Input", "RunTurnRate", m_dMovingFastDegreesPerSec, &m_dMovingFastDegreesPerSec);
	pPrefs->GetVal("Input", "StillSlowTurnRate", m_dStillSlowDegreesPerSec, &m_dStillSlowDegreesPerSec);
	pPrefs->GetVal("Input", "StillFastTurnRate", m_dStillFastDegreesPerSec, &m_dStillFastDegreesPerSec);
	pPrefs->GetVal("Input", "TapRotationDegrees", m_sTapRotationDegrees, &m_sTapRotationDegrees);
	
	pPrefs->GetVal("Input", "MouseSensitivityX", m_dMouseSensitivityX, &m_dMouseSensitivityX);
	pPrefs->GetVal("Input", "MouseSensitivityY", m_dMouseSensitivityY, &m_dMouseSensitivityY);

	pPrefs->GetVal("Input", "UseMouse", m_sUseMouse, &m_sUseMouse);


	pPrefs->GetVal("Input", "UseJoystick", m_sUseJoy, &m_sUseJoy);

	pPrefs->GetVal("Input", "JoyButtonMenuStart", 7, &m_sJoyStartButton);
	pPrefs->GetVal("Input", "JoyButtonMenuUp", 12, &m_sJoyMenuUpButton);
	pPrefs->GetVal("Input", "JoyButtonMenuDown", 13, &m_sJoyMenuDownButton);
	pPrefs->GetVal("Input", "JoyButtonMenuLeft", 14, &m_sJoyMenuLeftButton);
	pPrefs->GetVal("Input", "JoyButtonMenuRight", 15, &m_sJoyMenuRightButton);
	pPrefs->GetVal("Input", "JoyAxisMenuUp", 4, &m_sJoyMenuUpAxis);
	pPrefs->GetVal("Input", "JoyAxisMenuDown", 3, &m_sJoyMenuDownAxis);
	pPrefs->GetVal("Input", "JoyAxisMenuLeft", 2, &m_sJoyMenuLeftAxis);
	pPrefs->GetVal("Input", "JoyAxisMenuRight", 1, &m_sJoyMenuRightAxis);
	pPrefs->GetVal("Input", "JoyButtonMenuConfirm", 1, &m_sJoyMenuConfirmButton);
	pPrefs->GetVal("Input", "JoyButtonMenuBack", 2, &m_sJoyMenuBackButton);
	pPrefs->GetVal("Input", "JoyButtonMenuBack2", 5, &m_sJoyMenuBackButton2);
	pPrefs->GetVal("Input", "JoyButtonMenuDeleteKeybind", 5, &m_sJoyMenuDeleteKeybindButton);

	// Game play keys.
	short	i;
	char	szDescriptor[256];
	for (i = 0; i < NumInputFunctions; i++)
		{
		pPrefs->GetVal(
			"Keys", 
			ms_ainputinfo[i].pszSaveName,
			g_apszKeyDescriptions[m_asPlayKeys[i]],
			szDescriptor 
			);

		// Attempt to translate . . .
		if (KeyDescriptionToValue(szDescriptor, &(m_asPlayKeys[i]) ) != 0)
			{
			TRACE("LoadPrefs(): Failed to convert key description %s to key value.\n",
				szDescriptor);
			}

		pPrefs->GetVal(
			"Mouse",
			ms_ainputinfo[i].pszSaveName,
			g_apszMouseButtonDescriptions[MouseBitfieldToIndex(m_asPlayMouseButtons[i])],
			szDescriptor
			);

		// Attempt to translate . . .
		if (MouseButtonDescriptionToMask(szDescriptor, &(m_asPlayMouseButtons[i]) ) != 0)
			{
			TRACE("LoadPrefs(): Failed to convert mouse button description %s to button mask.\n",
				szDescriptor);
			}

		pPrefs->GetVal(
			"Joystick",
			ms_ainputinfo[i].pszSaveName,
			g_apszJoyButtonDescriptions[JoyBitfieldToIndex(m_asPlayJoyButtons[i])],
			szDescriptor
			);

		// Attempt to translate . . .
		if (JoyButtonDescriptionToMask(szDescriptor, &(m_asPlayJoyButtons[i]) ) != 0)
			{
			TRACE("LoadPrefs(): Failed to convert joy button description %s to button mask.\n",
				szDescriptor);
			}
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
short CInputSettings::SavePrefs(
	RPrefs* pPrefs)
	{
	pPrefs->SetVal("Input", "WalkTurnRate", m_dMovingSlowDegreesPerSec);
	pPrefs->SetVal("Input", "RunTurnRate", m_dMovingFastDegreesPerSec);
	pPrefs->SetVal("Input", "StillSlowTurnRate", m_dStillSlowDegreesPerSec);
	pPrefs->SetVal("Input", "StillFastTurnRate", m_dStillFastDegreesPerSec);
	
	pPrefs->SetVal("Input", "TapRotationDegrees", m_sTapRotationDegrees);

	pPrefs->SetVal("Input", "MouseSensitivityX", m_dMouseSensitivityX);
	pPrefs->SetVal("Input", "MouseSensitivityY", m_dMouseSensitivityY);

	pPrefs->SetVal("Input", "UseMouse", m_sUseMouse);

	pPrefs->SetVal("Input", "UseJoystick", m_sUseJoy);

	// Game play input.
	short	i;
	for (i = 0; i < NumInputFunctions; i++)
		{
		pPrefs->SetVal(
			"Keys", 
			ms_ainputinfo[i].pszSaveName,
			g_apszKeyDescriptions[m_asPlayKeys[i]] 
			);

		pPrefs->SetVal(
			"Mouse",
			ms_ainputinfo[i].pszSaveName,
			g_apszMouseButtonDescriptions[MouseBitfieldToIndex(m_asPlayMouseButtons[i])]
			);

		pPrefs->SetVal(
			"Joystick",
			ms_ainputinfo[i].pszSaveName,
			g_apszJoyButtonDescriptions[JoyBitfieldToIndex(m_asPlayJoyButtons[i])]
			);
		}

	return pPrefs->IsError();
	}

//////////////////////////////////////////////////////////////////////////////
// Load settings that are stored in game file
//////////////////////////////////////////////////////////////////////////////
short CInputSettings::LoadGame(
	RFile* pFile)
	{
	return 0;
	}


//////////////////////////////////////////////////////////////////////////////
// Save settings that are stored in game file
//////////////////////////////////////////////////////////////////////////////
short CInputSettings::SaveGame(
	RFile* pFile)
	{
	return 0;
	}


//////////////////////////////////////////////////////////////////////////////
// Temporarily set settings for demo mode (file is for saving current settings)
//////////////////////////////////////////////////////////////////////////////
short CInputSettings::PreDemo(
	RFile* pFile)
	{
	// Store current keys?

	return 0;
	}


//////////////////////////////////////////////////////////////////////////////
// Restore settings to what they were prior to demo mode
//////////////////////////////////////////////////////////////////////////////
short CInputSettings::PostDemo(
	RFile* pFile)
	{
	// Restore user keys?

	return 0;
	}


///////////////////////////////////////////////////////////////////////////////
// Set rotation values to the defaults.
///////////////////////////////////////////////////////////////////////////////
void CInputSettings::DefaultRotations(void)
	{
	m_dMovingSlowDegreesPerSec		= 240.0;
	m_dMovingFastDegreesPerSec		= 300.0;
	m_dStillSlowDegreesPerSec		= 180.0;
	m_dStillFastDegreesPerSec		= 240.0;
	m_sTapRotationDegrees			= 10;
	}

///////////////////////////////////////////////////////////////////////////////
// Internal functions.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
