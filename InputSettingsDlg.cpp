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
// InputSettingsDlg.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		07/03/97 JMI	Started.
//
//		07/03/97	JMI	Checking in for work at home.
//
//		07/04/97	JMI	Added a whole fancy (in that it's quite simple) way of
//							doing all the settings from one dialog with a scroll view
//							(a transparent listbox) but having them call together
//							makes the interface clunky for the mouse and the joystick.
//							So I'm checking this in to have it but I plan to rewrite
//							more the way Mike originally pictured which, as it turns
//							out, will probably be rather simple via the menus combined
//							with the current way of getting the input.
//
//		07/06/97	JMI	Added new version that uses the menu system for most
//							of the interface.
//							Also, changed m_asPlayButtons to m_asPlayMouseButtons.
//							Removed EditKeySettings() and EditMouseSetings().
//
//		07/07/97	JMI	Now checks to make sure the listen mode hasn't been
//							cancelled before listening for input in ListenForInput().
//
//		07/07/97	JMI	Now InputSettingsDlg_InitMenu() sets up the 'Go back'
//							(cancel) menu item no matter what so that, if it aborts
//							the menu for some reason, the menu system knows how to
//							go back to the previous.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/21/97	JMI	Was not setting the default GUIs' font.  This implied
//							it was relying on other things to set this.  Since this
//							can feasibly no longer happen before this menu/dlg comes
//							up, it now has to set the the font itself.
//
//		08/10/97	JMI	Now sets sEnabled in each dynamically created menu item.
//							Also, ASSERTs if not enough menu items.
//
//		08/17/97 MJR	Now uses g_resmgrShell to load its gui item.
//
//		08/18/97	JMI	Added ms_au8UnmappableKeys[] for the keys that the user
//							is not allowed to map to.
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem() and occurrences
//							of rspUpdateDisplay() to UpdateDisplay().
//
//		08/22/97	JMI	Changed calls to UpdateDisplay() back to rspUpdateDisplay()
//							since we no longer need UpdateDisplay() now that we are
//							using rspLock/Unlock* functions properly.
//
//		08/27/97	JMI	Now uses g_fontPostal instead of g_fontBig.
//
//		08/27/97	JMI	Now uses g_fontBig instead of g_fontPostal.
//
//		08/27/97	JMI	Now does not actually create a back item but sets the
//							cancel item to just beyond the last item.
//
//		08/27/97	JMI	The program no longer sets the text fore and shadow 
//							colors (also does not set the shadow effect).  Instead
//							uses all Artie's settings.
//
//		09/26/97	JMI	Now uses the 'Restore defaults' string from localize.
//
//		10/10/97	JMI	Filled in portions that poll joystick (since no event
//							driven stuff is yet available).
//
//		11/05/97	JMI	Converted to new joystick API.
//
//////////////////////////////////////////////////////////////////////////////
//
// Deals with input settings dialogs.  There is an interface for both the 
//	keyboard and mouse dialogs that deals heavily with user input, and GUI 
// output.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////
#include "RSPiX.h"

#include "game.h"
#include "update.h"
#include "InputSettings.h"
#include "input.h"
#include "keys.h"
#include "menus.h"
#include "InputSettingsDlg.h"

#include "CompileOptions.h"	// For ALLOW_JOYSTICK macro.

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )

#define INPUT_ITEM_GUI				"menu/InputItm.gui"

#define TEXT_LISTEN_COLOR			251

// How often to flash in ms.
#define FLASH_TIMEOUT				250

#define FONT_HEIGHT					15

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

static bool			ms_bListenForInput	= false;	// true, when listening for input
																// to set for current input function.

static U32			ms_u32OrigTextColor;

static bool			ms_bMenuDone			= false;	// true, when the current menu
																// is done.

static short		ms_sResetItem;						// Index of item that restores
																// the default inputs.
static short		ms_sResetItemOld;					// Index of item that restores the old default inputs.

static short		ms_sMouseButtons;
static short		ms_sJoyButtons;

static long			ms_lFlashTimer;

// This array contains RSP_SK_* macros for the keys that should be 
// inaccessible to the user when it comes to mapping the input keys.
// If you are adding a key to this array, PLEASE ADD THE _SK_ VERSION (not
// the _GK_ version).
static U8			ms_au8UnmappableKeys[]	=
	{
	RSP_SK_F1,				// Next level.
	RSP_SK_F2,				// Toggle targeting.
	RSP_SK_F3,				// XRay all.
	RSP_SK_F4,				// Display info.
	RSP_SK_F5,				// Mission goal.
	RSP_SK_F6,				// Reserved for future use.
	RSP_SK_F7,				// Reserved for future use.
	RSP_SK_F8,				// Reserved for future use.
	RSP_SK_F9,				// Reserved for future use.
	RSP_SK_F10,				// Reserved for future use.
	RSP_SK_F11,				// Show realm statistics (editor only).
	RSP_SK_F12,				// Reserved for future use.
	RSP_SK_MINUS,			// Reduce display area.
	RSP_SK_EQUALS,			// Increase display area.
	RSP_SK_NUMPAD_MINUS,	// Reduce display area.   
	RSP_SK_NUMPAD_PLUS,	// Increase display area. 
	RSP_SK_ENTER,			// Take snap shot.
	RSP_SK_PAUSE,			// Pause the game.
	RSP_SK_ESCAPE,			// Abort/Menu key.
	RSP_SK_CAPSLOCK,		// Toggle Run/Walk modages.
	};

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Internal Functions.
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Externally callable functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Called to setup an input settings menu.
//////////////////////////////////////////////////////////////////////////////
extern short InputSettingsDlg_InitMenu(	// Returns 0 on success.
	Menu* pmenu)									// In:  Menu to setup.
	{
	short		sRes				= 0;		// Assume success.
	short		sInputIndex		= 0;		// Safety.
	U32*	pasPlayInputs	= NULL;	// Input value array.
	char**	papszInputDescriptions	= NULL;	// Descriptions of input values.
	bool	bIsJoystick = false;

	switch (pmenu->u32Id)
		{
		case KEYBOARD_MENU_ID:
			// Use keys.
			pasPlayInputs				= g_InputSettings.m_asPlayKeys;
			papszInputDescriptions	= g_apszKeyDescriptions;
			break;
		case MOUSE_MENU_ID:
			// Use mouse buttons.
			pasPlayInputs				= g_InputSettings.m_asPlayMouseButtons;
			papszInputDescriptions	= g_apszMouseButtonDescriptions;
			break;
		case JOYSTICK_MENU_ID:
			// Use joy buttons.
			pasPlayInputs				= g_InputSettings.m_asPlayJoyButtons;
			papszInputDescriptions	= g_apszJoyButtonDescriptions;
			bIsJoystick = true;
			break;
		default:
			TRACE("InptuSettingsDlg_InitMenu(): Unsupported menu.\n");
			sRes	= -1;
			break;
		}

	// Assert we have enough room for the input functions and the 'defaults' and 'back'
	// menu items.
	ASSERT(CInputSettings::NumInputFunctions < (NUM_ELEMENTS(pmenu->ami) - 2) );

	// Set font for GUIs' default print.
	// Cell height doesn't matter since it is set by the GUIs themselves.
	RGuiItem::ms_print.SetFont(FONT_HEIGHT, &g_fontBig);

	for (sInputIndex = 0; sInputIndex < CInputSettings::NumInputFunctions && sRes == 0; sInputIndex++)
		{
		// Set text describing input function for this menu item.
		pmenu->ami[sInputIndex].pszText	= CInputSettings::ms_ainputinfo[sInputIndex].pszDescription;
		// Enable item.
		pmenu->ami[sInputIndex].sEnabled	= TRUE;
		// Load GUI for input method description.

		RGuiItem*	pgui;
		if (rspGetResourceInstance(&g_resmgrShell, INPUT_ITEM_GUI, (RTxt**)(&pgui)) == 0)
			{
			// Let the menu know about the GUI.
			pmenu->ami[sInputIndex].pgui	= pgui;
			// Set the GUIs text to the corresponding input description and recompose.
			if (bIsJoystick)
				pgui->SetText("%s", papszInputDescriptions[JoyBitfieldToIndex(pasPlayInputs[sInputIndex])]);
			else if (pmenu->u32Id == MOUSE_MENU_ID)
				pgui->SetText("%s", papszInputDescriptions[MouseBitfieldToIndex(pasPlayInputs[sInputIndex])]);
			else
				pgui->SetText("%s", papszInputDescriptions[pasPlayInputs[sInputIndex]]);
			// Remember associated input function index.
			pgui->m_ulUserData	= sInputIndex;
			// TEMP until the GUI editor can setup shadow text.
//			pgui->m_sTextEffects	= RGuiItem::Shadow;
//			pgui->m_u32TextColor			= GetCurrentMenuBox()->m_u32TextColor;
//			pgui->m_u32TextShadowColor	= GetCurrentMenuBox()->m_u32TextShadowColor;
			// END TEMP
			pgui->Compose();
			}
		else
			{
			TRACE("InputSettingsDlg_InitMenu(): LoadInstantiate() failed.\n");
			sRes	= -1;
			}
		}

	// Remember the reset item index.
	ms_sResetItem	= sInputIndex;

	// Add reset.
	pmenu->ami[sInputIndex].pszText	= g_pszRotationSetupMenu_RestoreDefaults;
	// Enable item.
	pmenu->ami[sInputIndex].sEnabled	= TRUE;

	// Next item please.
	sInputIndex++;

	// Remember the reset item index.
	ms_sResetItemOld = sInputIndex;

	// Add reset.
	pmenu->ami[sInputIndex].pszText = g_pszRotationSetupMenu_RestoreDefaultsOld;
	// Enable item.
	pmenu->ami[sInputIndex].sEnabled = TRUE;

	// Next item please.
	sInputIndex++;

	// Make the back the cancel item.
	pmenu->menuautoitems.sCancelItem	= sInputIndex;

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// Called to clean up an input settings menu.
//////////////////////////////////////////////////////////////////////////////
extern short InputSettingsDlg_KillMenu(	// Returns 0 on success.
	Menu* pmenu)									// In:  Menu to clean up.  
	{
	short	sRes	= 0;	// Assume success.

	short	sInputIndex;
	// Delete the loaded GUIs.
	for (sInputIndex = 0; sInputIndex < CInputSettings::NumInputFunctions && sRes == 0; sInputIndex++)
		{
		// Clean up the GUI.
		rspReleaseResourceInstance(&g_resmgrShell, &pmenu->ami[sInputIndex].pgui);
		}

	// Flag the looper that we're done.
	ms_bMenuDone	= true;

	return sRes;
	}


//////////////////////////////////////////////////////////////////////////////
// Called when a choice is made or a selection is changed on an input
// setttings menu.
//////////////////////////////////////////////////////////////////////////////
void InputSettingsDlg_Choice(	// Returns nothing.
	Menu*	pmenu,					// In:  Current menu.
	short sMenuItem)				// In:  Menu item chosen or -1 if selection 
										// change.
	{
	static U8*	pau8KeyStatusArray	= rspGetKeyStatusArray();

	short	sError	 = 0;

	if (sMenuItem > -1)
		{
		// If the specified item has a GUI . . .
		if (pmenu->ami[sMenuItem].pgui)
			{
			ms_bListenForInput	= true;
			// Clear input array for key type.
			memset(pau8KeyStatusArray, 0, 128);
			// Clear buttons status for mouse type.
			ms_sMouseButtons	= 0;
			// Clear buttons tatus for joy type.
			ms_sJoyButtons		= 0;
			// Get GUI.
			RGuiItem*	pgui	= pmenu->ami[sMenuItem].pgui;
			// Note original color.
			ms_u32OrigTextColor	= pgui->m_u32TextColor;
			// Reset flash timer.
			ms_lFlashTimer			= rspGetMilliseconds() + FLASH_TIMEOUT;

			switch (pmenu->u32Id)
				{
				case KEYBOARD_MENU_ID:
					break;
				case MOUSE_MENU_ID:
					g_InputSettings.m_asPlayMouseButtons[pgui->m_ulUserData]	= ms_sMouseButtons;
					pgui->SetText("%s", g_apszMouseButtonDescriptions[MouseBitfieldToIndex(ms_sMouseButtons)]);
					pgui->Compose();
					break;
				case JOYSTICK_MENU_ID:
					g_InputSettings.m_asPlayJoyButtons[pgui->m_ulUserData] = JoyIndexToBitfield(ms_sJoyButtons);
					pgui->SetText("%s", g_apszJoyButtonDescriptions[JoyIndexToBitfield(ms_sJoyButtons)]);
					pgui->Compose();
					break;
				}
			}
		else
			{
			if (sMenuItem == ms_sResetItem
				|| sMenuItem == ms_sResetItemOld)
				{
				U32*	pasPlayInputs	= NULL;				// Input value array.
				char**	papszInputDescriptions	= NULL;	// Descriptions of input values.

				switch (pmenu->u32Id)
					{
					case KEYBOARD_MENU_ID:
						// Use keys.
						pasPlayInputs				= g_InputSettings.m_asPlayKeys;
						papszInputDescriptions	= g_apszKeyDescriptions;
						break;
					case MOUSE_MENU_ID:
						// Use mouse buttons.
						pasPlayInputs				= g_InputSettings.m_asPlayMouseButtons;
						papszInputDescriptions	= g_apszMouseButtonDescriptions;
						break;
					case JOYSTICK_MENU_ID:
						// Use joy buttons.
						pasPlayInputs				= g_InputSettings.m_asPlayJoyButtons;
						papszInputDescriptions	= g_apszJoyButtonDescriptions;
						break;
					default:
						TRACE("InputSettingsDlg_Choice(): Unsupported menu.\n");
						sError	= 1;
						break;
					}

				short	sInputIndex;
				RGuiItem*	pgui;
				for (sInputIndex = 0; sInputIndex < CInputSettings::NumInputFunctions && sError == 0; sInputIndex++)
					{
					pgui	= pmenu->ami[sInputIndex].pgui;
					// Restore input default.
					switch (pmenu->u32Id)
						{
						case KEYBOARD_MENU_ID:
							// Use keys.
							pasPlayInputs[sInputIndex]			= sMenuItem == ms_sResetItemOld ? CInputSettings::ms_ainputinfoOld[sInputIndex].u8DefaultKey : CInputSettings::ms_ainputinfo[sInputIndex].u8DefaultKey;
							break;
						case MOUSE_MENU_ID:
							// Use mouse buttons.
							pasPlayInputs[sInputIndex]			= sMenuItem == ms_sResetItemOld ? CInputSettings::ms_ainputinfoOld[sInputIndex].sDefMouseButtons : CInputSettings::ms_ainputinfo[sInputIndex].sDefMouseButtons;
							break;
						case JOYSTICK_MENU_ID:
							// Use joy buttons.
							pasPlayInputs[sInputIndex]			= sMenuItem == ms_sResetItemOld ? CInputSettings::ms_ainputinfoOld[sInputIndex].sDefJoyButtons : CInputSettings::ms_ainputinfo[sInputIndex].sDefJoyButtons;
							break;
						}
						
					// Set the GUIs text to the corresponding input description and recompose.
					if (pmenu->u32Id == JOYSTICK_MENU_ID)
						pgui->SetText("%s", g_apszJoyButtonDescriptions[JoyBitfieldToIndex(pasPlayInputs[sInputIndex])]);
					else if (pmenu->u32Id == MOUSE_MENU_ID)
						pgui->SetText("%s", g_apszMouseButtonDescriptions[MouseBitfieldToIndex(pasPlayInputs[sInputIndex])]);
					else
						pgui->SetText("%s", papszInputDescriptions[pasPlayInputs[sInputIndex]]);
					pgui->Compose();
					}
				}
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Checks is a key is mappable as an input via ms_au8UnmappableKeys[].
//////////////////////////////////////////////////////////////////////////////
static bool IsMappable(	// Returns true, if mappable, false otherwise.
	U8	u8Key)				// In:  Key to check mappable status on.
	{
	bool	bMappable	= true;

	short	sIndex;
	for (sIndex = 0; sIndex < NUM_ELEMENTS(ms_au8UnmappableKeys); sIndex++)
		{
		// If this is the specified key . . .
		if (ms_au8UnmappableKeys[sIndex] == u8Key)
			{
			// Not mappable.
			bMappable	= false;
			break;
			}
		}

	return bMappable;
	}

//////////////////////////////////////////////////////////////////////////////
// Listens for input.  If input detected, it is updated to the appropriate
// input array.
//////////////////////////////////////////////////////////////////////////////
inline void ListenForInput(	// Returns nothing.
	RInputEvent*	pie)			// In:  Input event.
	{
	static U8*	pau8KeyStatusArray	= rspGetKeyStatusArray();

	RGuiItem*	pgui	= RGuiItem::ms_pguiFocus;
	bool			bAffirmitiveFeedback	= true;	// true to play affirmitive audible
															// feedback, false for negative.

	switch (pie->type)
		{
		case RInputEvent::Key:
			if (pie->lKey == 27)
				{
				ms_bListenForInput	= false;
				bAffirmitiveFeedback	= false;
				}

			// Don't allow menu to respond to any key events.
			pie->sUsed	= TRUE;

			break;
		}

	// If still listening . . .
	if (ms_bListenForInput == true)
		{
		switch (GetCurrentMenu()->u32Id)
			{
			case KEYBOARD_MENU_ID:
				{
				U8*	pu8Key = pau8KeyStatusArray;
				short	i;
				for (i = 0; i < 128; i++, pu8Key++)
					{
					// If pressed . . .
					if ((*pu8Key) & 1)
						{
						// If mappable . . .
						if (IsMappable(i) == true)
							{
							pgui->SetText("%s", g_apszKeyDescriptions[i]);
							pgui->Compose();

							g_InputSettings.m_asPlayKeys[pgui->m_ulUserData]	= i;
							}
						else
							{
							bAffirmitiveFeedback	= false;
							}

						ms_bListenForInput	= false;

						break;
						}
					}
				break;
				}
			case MOUSE_MENU_ID:
				 switch (pie->type)
					{
					case RInputEvent::Key:
						if (pie->lKey == '\r')
							{
							// Done with mode.
							ms_bListenForInput	= false;

							g_InputSettings.m_asPlayMouseButtons[pgui->m_ulUserData]	= ms_sMouseButtons;

							pgui->SetText("%s", g_apszMouseButtonDescriptions[ms_sMouseButtons]);
							pgui->Compose();
							}
						break;
					case RInputEvent::Mouse:
						// Switch on type (up or down) . . .
						switch (pie->sEvent)
							{
							case RSP_MB0_PRESSED:
							case RSP_MB1_PRESSED:
							case RSP_MB2_PRESSED:
							case 1025: // Yeah I dunno
							case 1027:
								// Stay in mode, just update status.
								ms_sMouseButtons	|= pie->sButtons;
								break;

							case RSP_MB0_RELEASED:
							case RSP_MB1_RELEASED:
							case RSP_MB2_RELEASED:
							case 1026: // Yeah I dunno
								// Done with mode.
								ms_bListenForInput	= false;

								g_InputSettings.m_asPlayMouseButtons[pgui->m_ulUserData] = ms_sMouseButtons;
								break;
							}
						
						pgui->SetText("%s", g_apszMouseButtonDescriptions[MouseBitfieldToIndex(ms_sMouseButtons)]);
						pgui->Compose();
						break;
					}
				break;
			case JOYSTICK_MENU_ID:
				 switch (pie->type)
					{
					case RInputEvent::Key:
						if (pie->lKey == '\r')
							{
							// Done with mode.
							ms_bListenForInput	= false;

							g_InputSettings.m_asPlayJoyButtons[pgui->m_ulUserData] = JoyIndexToBitfield(ms_sJoyButtons);

							pgui->SetText("%s", g_apszJoyButtonDescriptions[JoyIndexToBitfield(ms_sJoyButtons)]);
							pgui->Compose();
							}
						break;
					case RInputEvent::None:
						{
#if defined(ALLOW_JOYSTICK)
						XInputState xis = {};
						GetXInputState(&xis);
						for (int i = 0; i < XInputButtons; i++)
						{
							// If button was released...
							if (xis.ButtonState[i] == XInputState::Press)
							{
								// Done with mode.
								ms_bListenForInput	= false;
								g_InputSettings.m_asPlayJoyButtons[pgui->m_ulUserData]	= JoyIndexToBitfield(i);
								pgui->SetText("%s", g_apszJoyButtonDescriptions[i]);
								pgui->Compose();
								break;
							}
						}

#endif
#if 0 //defined(ALLOW_JOYSTICK)
						// Only use one joystick.
						rspUpdateJoy(0);

						// Get prev and current status.
						U32	u32ButtonsPrev;
						U32	u32ButtonsCur;
						rspGetJoyPrevState(0, &u32ButtonsPrev);
						rspGetJoyState(0, &u32ButtonsCur);

						// If less buttons are down . . .
						if (	(u32ButtonsPrev & RSP_JOY_BUT_1) > (u32ButtonsCur & RSP_JOY_BUT_1)
							||	(u32ButtonsPrev & RSP_JOY_BUT_2) > (u32ButtonsCur & RSP_JOY_BUT_2)
							||	(u32ButtonsPrev & RSP_JOY_BUT_3) > (u32ButtonsCur & RSP_JOY_BUT_3)
							||	(u32ButtonsPrev & RSP_JOY_BUT_4) > (u32ButtonsCur & RSP_JOY_BUT_4)
							||	(u32ButtonsPrev & RSP_JOY_BUT_5) > (u32ButtonsCur & RSP_JOY_BUT_5)
							||	(u32ButtonsPrev & RSP_JOY_BUT_6) > (u32ButtonsCur & RSP_JOY_BUT_6)
							||	(u32ButtonsPrev & RSP_JOY_BUT_7) > (u32ButtonsCur & RSP_JOY_BUT_7)
							||	(u32ButtonsPrev & RSP_JOY_BUT_8) > (u32ButtonsCur & RSP_JOY_BUT_8)
							||	(u32ButtonsPrev & RSP_JOY_BUT_9) > (u32ButtonsCur & RSP_JOY_BUT_9)
							||	(u32ButtonsPrev & RSP_JOY_BUT_10) > (u32ButtonsCur & RSP_JOY_BUT_10)
							||	(u32ButtonsPrev & RSP_JOY_BUT_11) > (u32ButtonsCur & RSP_JOY_BUT_11)
							||	(u32ButtonsPrev & RSP_JOY_BUT_12) > (u32ButtonsCur & RSP_JOY_BUT_12)
							||	(u32ButtonsPrev & RSP_JOY_BUT_13) > (u32ButtonsCur & RSP_JOY_BUT_13)
							||	(u32ButtonsPrev & RSP_JOY_BUT_14) > (u32ButtonsCur & RSP_JOY_BUT_14)
							||	(u32ButtonsPrev & RSP_JOY_BUT_15) > (u32ButtonsCur & RSP_JOY_BUT_15)
							||	(u32ButtonsPrev & RSP_JOY_BUT_16) > (u32ButtonsCur & RSP_JOY_BUT_16)
							)
							{
							// Done with mode.
							ms_bListenForInput	= false;

							g_InputSettings.m_asPlayJoyButtons[pgui->m_ulUserData]	= JoyIndexToBitfield(ms_sJoyButtons);
							}
						else
							{
							// Stay in mode, just update status.  Limit to four buttons due to
							// limitations imposed when joystick interface was four buttons.  Next time
							// we'll allow many, many buttons.
							//ms_sJoyButtons	= (short)(u32ButtonsCur & (RSP_JOY_BUT_1 | RSP_JOY_BUT_2 | RSP_JOY_BUT_3 | RSP_JOY_BUT_4) );

							ms_sJoyButtons = JoyBitfieldToIndex(u32ButtonsCur);

							// Don't allow binding "Start" because we need it to pull up the menu.
							if (ms_sJoyButtons == g_InputSettings.m_sJoyStartButton)
								ms_sJoyButtons = 0;

							}
						
						// Sanity check
						if (ms_sJoyButtons >= 0 && ms_sJoyButtons < 16)
							pgui->SetText("%s", g_apszJoyButtonDescriptions[ms_sJoyButtons]);
						else
							pgui->SetText("???");
						pgui->Compose();
#endif	// defined(ALLOW_JOYSTICK)
						break;
						}
					}
				break;
			}
		}

	// If mode completed . . .
	if (ms_bListenForInput == false)
		{
		if (bAffirmitiveFeedback == true)
			{
			PlaySample(g_smidLoadedWeapon, SampleMaster::Unspecified);
			}
		else
			{
			PlaySample(g_smidEmptyWeapon, SampleMaster::Unspecified);
			}

		// Make sure text color is restored.
		pgui->m_u32TextColor	= ms_u32OrigTextColor;
		pgui->Compose();
		}
	else
		{
		// If timer has expired . . .
		if (rspGetMilliseconds() > ms_lFlashTimer)
			{
			// Flash item.
			if (pgui->m_u32TextColor == ms_u32OrigTextColor)
				{
				pgui->m_u32TextColor	= TEXT_LISTEN_COLOR;
				}
			else
				{
				pgui->m_u32TextColor	= ms_u32OrigTextColor;
				}

			pgui->Compose();

			// Reset flash timer.
			ms_lFlashTimer			= rspGetMilliseconds() + FLASH_TIMEOUT;
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Edit the input settings via menu.
//////////////////////////////////////////////////////////////////////////////
extern short EditInputSettings(void)	// Returns nothing.
	{
	short	sRes	= 0;	// Assume success.
	bool bDeleteKeybind = false;	// If true, we want to delete the keybind we're on.

	// Menu is already started.

	// Menu is only just starting.
	ms_bMenuDone = false;

	RInputEvent	ie;

	// Until the menu exits.
	while (rspGetQuitStatus() == FALSE && ms_bMenuDone == false)
		{
		// Update the system, drive the audio, blah blah blah.
		UpdateSystem();

		ie.type	= RInputEvent::None;
		rspGetNextInputEvent(&ie);

		switch (ie.type)
			{
			case RInputEvent::Key:
				if (ie.lKey == RSP_SK_BACKSPACE)
					bDeleteKeybind = true;
				break;
			case RInputEvent::Mouse:
				switch (ie.sEvent)
					{
					case RSP_MB0_PRESSED:
					case RSP_MB1_PRESSED:
					case RSP_MB2_PRESSED:
						if (RGuiItem::ms_pguiFocus && ms_bListenForInput == false)
							{
							// Activate listen mode.
							InputSettingsDlg_Choice(
								GetCurrentMenu(),
								RGuiItem::ms_pguiFocus->m_ulUserData);
							}
						break;
					}
				break;
			case RInputEvent::None:
#if defined(ALLOW_JOYSTICK)
				if (RGuiItem::ms_pguiFocus && ms_bListenForInput == false)
					{
#if 0
					// Only use one joystick.
					rspUpdateJoy(0);

					// Get prev and current status.
					U32	u32ButtonsPrev;
					U32	u32ButtonsCur;
					rspGetJoyPrevState(0, &u32ButtonsPrev);
					rspGetJoyState(0, &u32ButtonsCur);

					// If less buttons are up . . .
					if (	(u32ButtonsCur & RSP_JOY_BUT_1) > (u32ButtonsPrev & RSP_JOY_BUT_1)
						||	(u32ButtonsCur & RSP_JOY_BUT_2) > (u32ButtonsPrev & RSP_JOY_BUT_2)
						||	(u32ButtonsCur & RSP_JOY_BUT_3) > (u32ButtonsPrev & RSP_JOY_BUT_3)
						||	(u32ButtonsCur & RSP_JOY_BUT_4) > (u32ButtonsPrev & RSP_JOY_BUT_4)
						||	(u32ButtonsCur & RSP_JOY_BUT_5) > (u32ButtonsPrev & RSP_JOY_BUT_5)
						||	(u32ButtonsCur & RSP_JOY_BUT_6) > (u32ButtonsPrev & RSP_JOY_BUT_6)
						||	(u32ButtonsCur & RSP_JOY_BUT_7) > (u32ButtonsPrev & RSP_JOY_BUT_7)
						||	(u32ButtonsCur & RSP_JOY_BUT_8) > (u32ButtonsPrev & RSP_JOY_BUT_8)
						||	(u32ButtonsCur & RSP_JOY_BUT_9) > (u32ButtonsPrev & RSP_JOY_BUT_9)
						||	(u32ButtonsCur & RSP_JOY_BUT_10) > (u32ButtonsPrev & RSP_JOY_BUT_10)
						||	(u32ButtonsCur & RSP_JOY_BUT_11) > (u32ButtonsPrev & RSP_JOY_BUT_11)
						||	(u32ButtonsCur & RSP_JOY_BUT_12) > (u32ButtonsPrev & RSP_JOY_BUT_12)
						||	(u32ButtonsCur & RSP_JOY_BUT_13) > (u32ButtonsPrev & RSP_JOY_BUT_13)
						||	(u32ButtonsCur & RSP_JOY_BUT_14) > (u32ButtonsPrev & RSP_JOY_BUT_14)
						||	(u32ButtonsCur & RSP_JOY_BUT_15) > (u32ButtonsPrev & RSP_JOY_BUT_15)
						||	(u32ButtonsCur & RSP_JOY_BUT_16) > (u32ButtonsPrev & RSP_JOY_BUT_16)
						)						
						{
						// Activate listen mode.
						InputSettingsDlg_Choice(
							GetCurrentMenu(),
							RGuiItem::ms_pguiFocus->m_ulUserData);
						}
#endif
					}
#endif	// defined(ALLOW_JOYSTICK)
				break;
			}

		// If listening . . .
		if (ms_bListenForInput == true)
			{
			ListenForInput(&ie);
			}

		// Let menu system do its thing
		DoMenuInput(&ie, 1);
		DoMenuOutput(g_pimScreenBuf);

		// Joystick: erase keybind with Back button
		XInputState xis = {};
		GetLastXInputState(&xis);
		if (xis.ButtonState[g_InputSettings.m_sJoyMenuDeleteKeybindButton] == XInputState::Press)
		{
			bDeleteKeybind = true;
			GetXInputState(&xis); // throw out this state
		}

		if (bDeleteKeybind)
		{
			RGuiItem*	pgui = RGuiItem::ms_pguiFocus;
			if (pgui)
			{
				if (GetCurrentMenu()->u32Id == KEYBOARD_MENU_ID)
					g_InputSettings.m_asPlayKeys[pgui->m_ulUserData] = 0;
				else if (GetCurrentMenu()->u32Id == MOUSE_MENU_ID)
					g_InputSettings.m_asPlayMouseButtons[pgui->m_ulUserData] = 0;
				else if (GetCurrentMenu()->u32Id == JOYSTICK_MENU_ID)
					g_InputSettings.m_asPlayJoyButtons[pgui->m_ulUserData] = 0;
				pgui->SetText("None");
				pgui->Compose();
			}
			bDeleteKeybind = false;
		}

		// Update the screen
		rspUpdateDisplay();
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
