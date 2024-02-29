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
// input.cpp
// Project: Postal
//
// This module deals with getting user input.
//
// History:
//		12/05/96 MJR	Started.
//
//		03/24/97	JMI	Changed m_aInputs[] from long to UINPUT.
//
//		03/31/97	JMI	Added g_InputSettings instantiation.
//
//		04/11/97 MJR	Moved lots of stuff into input.cpp and added support for
//							recording and playing back inputs for self-playing mode.
//
//		04/11/97	JMI	Fixed typo with comparison in SetInputMode().
//							Also, added event driven keys.
//							Also, no user input allowed in release mode **this is temp
//							for ABC demo**.
//
//		04/24/97	JMI	Moved around INPUT_JUMP, INPUT_SUICIDE, and 
//							INPUT_NEXT_LEVEL.
//							Also, added INPUT_WEAPON_6...9 and INPUT_EXECUTE.  
//							Consequently, we're out of inputs (and we still don't have
//							INPUT_TOGGLE_HUMANSHIELD).
//
//		04/25/97	JMI	Forgot to add check for INPUT_EXECUTE on last update.
//
//		05/02/97	JMI	Increased number of available input bits by reducing
//							the weapon changing bits to 4 bits (now a number instead of
//							bit fields).
//
//		05/13/97	JMI	Changed INPUT_RUN to INPUT_WALK.
//							Changed Suicide and NextLevel to polling instead of event
//							driven.
//
//		05/14/97	JMI	Added INPUT_PICKUP.
//
//		06/06/97	JMI	Now uses rspGetKeyStatusArray() instead of rspScanKeys().
//							The rspGetKeyStatusArray() method is much more likely to
//							catch keys that get missed by polling, but, better than
//							rpsGetKey(), this function works on a key basis rather than
//							a key combo basis (i.e., SHIFT-1 is viewed as SHIFT and '1'
//							with rspGetKeyStatusArray() (like rspScanKeys()) but with
//							rspGetKey() it is seen as '!').
//
//		06/09/97	JMI	Changed from IS_INPUT to WAS_INPUT for Jump, 
//							Execute, Suicide, NextLevel, and PickUp.
//
//		06/09/97	JMI	Added tap rotation when standing.
//
//		06/09/97	JMI	Changed 'Walk' to 'Run'.
//
//		06/10/97	JMI	Changed to use all the new variations of user settable
//							rotation.
//
//		06/12/97	JMI	Added temp version of cheat keys that utilizes backspace
//							in combination with the normal weapon keys 1 - 5.
//
//		06/15/97 MJR	Removed INPUT_NEXT_LEVEL, which has now been replaced by
//							an client/server mechanism.
//							Changed the input modes from macros to enums.
//							Added function to delete demo data buffer.
//
//		06/15/97 MJR	Changed to separate InputDemoInit() and InputDemoKill()
//							functions to make it real obvious and clean.
//							If you fill the entire record buffer, the dude will
//							automatically commit suicide as the last thing he does.
//
//		07/06/97	JMI	Changed m_au8PlayKeys[] from a U8 array to a short array,
//							m_asPlayKeys[].
//							Also, changed m_asPlayButtons to m_asPlayMouseButtons.
//
//		07/07/97	JMI	Now uses g_InputSettings.m_dMouseSensitivityX,Y to tweak
//							the mouse input values before interpretation by the normal 
//							input rotational logic.
//
//		07/16/97	JMI	Added input for weapon ten reducing the number of cheats to
//							4.
//
//		07/19/97	JMI	Added InitLocalInput().  Also, changed inputLast which was
//							a static local to GetLocalInput() to file scope 
//							(ms_inputLastLocal) so it could be altered by other 
//							functions.
//
//		07/23/97	JMI	Changed cheats to key combos.  See ms_acheats to find out
//							what you have to type.
//
//		07/24/97	JMI	Now the cheat keys are NOT stored in the EXE as exactly
//							as they are typed to discourage the possibility people can
//							find them easily by viewing/searching the EXE.
//
//		07/25/97	JMI	Added more INPUT_*'s and changed INPUT_WEAPON_11,12,13,14
//							to INPUT_CHEAT_11,12,13,14.
//							Also, removed INPUT_PICKUP.  There are now 20 possible
//							cheat inputs.
//
//		07/30/97	JMI	Added INPUT_CHEAT_16.
//
//		08/04/97	JMI	Originally setup the Y mouse sensitivity so it work 
//							backwards (i.e., a higher number was less sensitive).
//							Fixed.
//
//		08/05/97	JMI	Modified FindCheatCombos() to search until no cheat key has
//							been found.
//
//		08/06/97	JMI	Changed InitLocalInput() to ClearLocalInput().
//
//		08/07/97	JMI	Added INPUT_CHEAT_17.
//
//		08/10/97	JMI	Added StrafeLeft and StrafeRight inputs.
//							Also, changed Jump to Revive and INPUT_JUMP to INPUT_REVIVE.
//
//		08/12/97	JMI	Now requires an input event for cheats for GetLocalInput().
//							Anagrams will no longer work and it will not be liberal
//							about additional keys mixed in (e.g., HONC would not work
//							for HOC).
//
//		08/12/97	JMI	Now the run key becomes the walk key when the caps lock
//							is on.
//							Also added support for Run2 and Strafe2 and removed 
//							StrafeLeft and StrafeRight.
//
//		08/17/97	JMI	Changed MAX_CONSEQ_CHEAT_KEY_LAG from 1000 to 2000.
//
//		08/18/97	JMI	Renamed almost all existing cheats and added new ones.
//							There is one empty cheat slot.
//
//		08/18/97	JMI	Added cheat code for sales people.
//
//		08/18/97	JMI	Added filler for non-sales cheat code so it wouldn't make
//							a cheat feedback sound when you simply typed 'sell'.
//
//		08/20/97	JMI	Changed cheat 29 for advance level.
//
//		08/26/97 BRH	Added a query function to see if the demo is done.
//
//		08/27/97	JMI	Added Fire2.
//
//		08/28/97	JMI	Added safety initializations for dRate in GetLocalInput().
//							Should not be necessary if no logic errors, but what the
//							hey?!
//
//		09/03/97	JMI	Now compiles in the new or the old cheats based on 
//							USE_NEW_CHEATS.
//
//		09/06/97 MJR	Changed ClearLocalInput() to use INPUT_IDLE.
//
//		09/17/97	JMI	Added DOS-for-Dummies (or 'Paloma') cheats for the editors'
//							demo as a conditional compile option.
//
//		09/24/97	JMI	Now does not set INPUT_EXECUTE when LOCALE == UK.
//
//		09/24/97	JMI	Now only sets INPUT_EXECUTE when LOCALE == US.
//
//		10/10/97	JMI	Now uses joystick settings if ALLOW_JOYSTICK is defined.
//
//		11/05/97	JMI	Converted to new joystick API.
//
//		12/04/97	JMI	Reduced horizontal mouse sensitivity by a factor of 3.
//
//		12/08/97	JMI	Casting order was causing a rightward motion bias that was
//							previously only notable on the Alpha at high frame rates
//							but, now that we exaggerated the mouse sensitivity values,
//							it is more obvious at lower frame rates.
//
//		09/27/99	JMI	Changed to allow execution only in any locale 
//							satisfying the CompilerOptions macro VIOLENT_LOCALE.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "input.h"
#include "CompileOptions.h"

#ifdef MOBILE
#include "android/android.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Location to set mouse to
#define MOUSE_RESET_X		320
#define MOUSE_RESET_Y		240

// Threshold of mouse Y movement that is unnoticed by the input module.
// More movement than this is considered an indication to move the dude
// forward or backward.
#define MOUSE_Y_THRESH		40

// Check for the specified input.
// This equates to:  is the key for this input pressed or, IF there are mouse 
// buttons for this input, are the mouse buttons in that combination?
#define IS_INPUT(input)																														\
	(	(pu8KeyStatus[g_InputSettings.m_asPlayKeys[input]]	& 1)																	\
	||	((sButtons & g_InputSettings.m_asPlayMouseButtons[input]) == g_InputSettings.m_asPlayMouseButtons[input]	\
		&& g_InputSettings.m_asPlayMouseButtons[input]	!= 0) IS_JOY_INPUT(input) )

// This equates to:  was the key for this input pressed or, IF there are mouse 
// buttons for this input, are the mouse buttons in that combination?
// You should use CLEAR_INPUT(input) with this version.
#define WAS_INPUT(input)																													\
	(	pu8KeyStatus[g_InputSettings.m_asPlayKeys[input]]																			\
	||	((sButtons & g_InputSettings.m_asPlayMouseButtons[input]) == g_InputSettings.m_asPlayMouseButtons[input]	\
		&& g_InputSettings.m_asPlayMouseButtons[input]	!= 0) IS_JOY_INPUT(input) )

// Clear the specified input.
#define CLEAR_INPUT(input)																													\
	pu8KeyStatus[g_InputSettings.m_asPlayKeys[input]] = 0

#if defined(ALLOW_JOYSTICK)
	#define IS_JOY_INPUT(input)																											\
		||	((u32Buttons & g_InputSettings.m_asPlayJoyButtons[input]) == g_InputSettings.m_asPlayJoyButtons[input]	\
			&& g_InputSettings.m_asPlayJoyButtons[input]	!= 0)
#else
	#define IS_JOY_INPUT(input)
#endif	// defined(ALLOW_JOYSTICK)

// Minimum memory buffer to use for recording input data.  To figure out a
// reasonable size, multiply a high number of frames per second by a large
// number of total seconds.
#define BUF_MAX_ENTRIES		(30 * (5 * 60))

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )

#define MAX_CONSEQ_CHEAT_KEY_LAG	2000	// In ms.

// Tweak characters of a string so they are tough to find in the exe.
// Use CHAR_UNTWEAK() to get a specific char back.
#define STR_TWEAK(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19)	\
	{								\
	!c0 ? 0 : c0 + 1,			\
	!c1 ? 0 : c1 + 2,			\
	!c2 ? 0 : c2 + 3,			\
	!c3 ? 0 : c3 + 4,			\
	!c4 ? 0 : c4 + 5,			\
	!c5 ? 0 : c5 + 6,			\
	!c6 ? 0 : c6 + 7,			\
	!c7 ? 0 : c7 + 8,			\
	!c8 ? 0 : c8 + 9,			\
	!c9 ? 0 : c9 + 10,		\
	!c10 ? 0 : c10 + 11,		\
	!c11 ? 0 : c11 + 12,		\
	!c12 ? 0 : c12 + 13,		\
	!c13 ? 0 : c13 + 14,		\
	!c14 ? 0 : c14 + 15,		\
	!c15 ? 0 : c15 + 16,		\
	!c16 ? 0 : c16 + 17,		\
	!c17 ? 0 : c17 + 18,		\
	!c18 ? 0 : c18 + 19,		\
	!c19 ? 0 : c19 + 20,		\
	'\0'							\
	}

// Detweak a char from a string previously tweaked by STR_TWEAK().
#define DETWEAK_CHAR(str, i)	(str[i] - (i + 1) )

typedef struct
	{
	char	szCheat[21];
	UINPUT	input;
	long	lLastValidInputTime;
	short	sCurrentIndex;
	} Cheat;

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Data for each dude
UINPUT m_aInputs[INPUT_MAX_DUDES];

// Last input for the local dude.
UINPUT	ms_inputLastLocal	= 0;

// Global input settings
CInputSettings	g_InputSettings;

// Current mode
INPUT_MODE m_mode;

// Buffer-related stuff
U32* m_pBuf = 0;				// Pointer to buffer. Must be a U32 to maintain demo compatibility!
long m_lBufIndex;					// Current index into buffer
long m_lBufEntries;				// Total entries in buffer

// Cheat structs.
// Add one plus the index of each string item so it's not recognizable when 
// searching/viewing the exe.  The chars will be untweaked when the cheat is 
// evaluated at runtime.
static Cheat	ms_acheats[]	=
	{
#if defined(USE_LA_PALOMA_CHEATS)
						 
		{ STR_TWEAK('B', 'E', 'E', 'R', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_11 },
		{ STR_TWEAK('V', 'E', 'S', 'T', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_12 },
		{ STR_TWEAK('P', 'A', 'C', 'K', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_13 },
		{ STR_TWEAK('S', 'T', 'U', 'F', 'F', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_14 },
		{ STR_TWEAK('R', 'E', 'V', 'I', 'V', 'E', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_15 },
		{ { 'D'+1, 'A'+2, 'W'+3, 'H'+4, 'O'+5, 'L'+6, 'E'+7, 'E'+8, 'N'+9, 'C'+10, 'H'+11, 'I'+12, 'L'+13, 'A'+14, 'D'+15, 'A'+16, }, INPUT_CHEAT_16, },
		{ { 'B'+1, 'R'+2, 'E'+3, 'A'+4, 'K'+5, 'Y'+6, 'O'+7, 'S'+8, 'A'+9, 'K'+10,				}, INPUT_CHEAT_17, },
		{ STR_TWEAK('M', 'Y', 'T', 'E', 'A', 'M', 'O', 'U', 'S', 'E', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_18 },
		{ STR_TWEAK('S', 'H', 'E', 'L', 'L', 'S', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_19 },
		{ STR_TWEAK('B', 'O', 'O', 'M', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_20 },
		{ STR_TWEAK('F', 'L', 'A', 'M', 'A', 'G', 'E', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_21 },
		{ STR_TWEAK('S', 'H', 'O', 'T', 'G', 'U', 'N', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_22 },
		{ STR_TWEAK('C', 'A', 'N', 'N', 'O', 'N', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_23 },
		{ STR_TWEAK('L', 'O', 'B', 'B', 'E', 'R', 'S', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_24 },
		{ STR_TWEAK('M', 'I', 'S', 'S', 'I', 'L', 'E', 'S', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_25 },
		{ STR_TWEAK('N', 'A', 'P', 'A', 'L', 'M', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_26 },
		{ STR_TWEAK('F', 'L', 'A', 'M', 'E', 'R', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_27 },
		{ STR_TWEAK('M', 'I', 'N', 'E', 'S', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_28 },
		{ STR_TWEAK('N', 'E', 'X', 'T', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_29 },
		{ STR_TWEAK('G', 'A', 'W', 'D', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'), INPUT_CHEAT_30 },
	

#else
			{ { 'H'+1, 'E'+2, 'A'+3, 'L'+4, 'T'+5, 'H'+6, 'F'+7, 'U'+8, 'L'+9,						},	INPUT_CHEAT_11, },
			{ { 'T'+1, 'H'+2, 'I'+3, 'C'+4, 'K'+5, 'S'+6, 'K'+7, 'I'+8, 'N'+9,						},	INPUT_CHEAT_12, },
			{ { 'C'+1, 'A'+2, 'R'+3, 'R'+4, 'Y'+5, 'M'+6, 'O'+7, 'R'+8, 'E'+9,						},	INPUT_CHEAT_13, },
	#if defined(USE_NEW_CHEATS)
 			{ { 'G'+1, 'I'+2, 'M'+3, 'M'+4, 'E'+5, 'D'+6, 'A'+7, 'T'+8,									},	INPUT_CHEAT_14, },
			{ { 'H'+1, 'E'+2, 'S'+3, 'S'+4, 'T'+5, 'I'+6, 'L'+7, 'L'+8, 'G'+9, 'O'+10,	'O'+11, 'D'+12,	},	INPUT_CHEAT_15, },
	#else
 			{ { 'M'+1, 'E'+2, 'G'+3, 'A'+4, 'S'+5, 'T'+6, 'U'+7, 'F'+8, 'F'+9,						},	INPUT_CHEAT_14, },
			{ { 'G'+1, 'E'+2, 'T'+3, 'U'+4, 'P'+5, 'R'+6, 'O'+7, 'G'+8, 'U'+9, 'E'+10,				},	INPUT_CHEAT_15, },
	#endif
			{ { 'D'+1, 'A'+2, 'W'+3, 'H'+4, 'O'+5, 'L'+6, 'E'+7, 'E'+8, 'N'+9, 'C'+10, 'H'+11, 'I'+12, 'L'+13, 'A'+14, 'D'+15, 'A'+16, }, INPUT_CHEAT_16, },
			{ { 'B'+1, 'R'+2, 'E'+3, 'A'+4, 'K'+5, 'Y'+6, 'O'+7, 'S'+8, 'A'+9, 'K'+10,				}, INPUT_CHEAT_17, },
			{ { 'M'+1, 'Y'+2, 'T'+3, 'E'+4, 'A'+5, 'M'+6, 'O'+7, 'U'+8, 'S'+9, 'E'+10,				}, INPUT_CHEAT_18, },
			{ { 'S'+1, 'H'+2, 'E'+3, 'L'+4, 'L'+5, 'F'+6, 'E'+7, 'S'+8, 'T'+9,						}, INPUT_CHEAT_19, },
			{ { 'E'+1, 'X'+2, 'P'+3, 'L'+4, 'O'+5, 'D'+6, 'A'+7, 'R'+8, 'A'+9, 'M'+10, 'A'+11,	}, INPUT_CHEAT_20, },
			{ { 'F'+1, 'L'+2, 'A'+3, 'M'+4, 'E'+5, 'N'+6, 'S'+7, 'T'+8, 'E'+9, 'I'+10, 'N'+11,	}, INPUT_CHEAT_21, },
			{ { 'S'+1, 'H'+2, 'O'+3, 'T'+4, 'G'+5, 'U'+6, 'N'+7,											}, INPUT_CHEAT_22, },
			{ { 'T'+1, 'H'+2, 'E'+3, 'B'+4, 'E'+5, 'S'+6, 'T'+7, 'G'+8, 'U'+9, 'N'+10,				}, INPUT_CHEAT_23, },
			{ { 'L'+1, 'O'+2, 'B'+3, 'I'+4, 'T'+5, 'F'+6, 'A'+7, 'R'+8,									}, INPUT_CHEAT_24, },
			{ { 'T'+1, 'I'+2, 'T'+3, 'A'+4, 'N'+5, 'I'+6, 'I'+7, 'I'+8,									}, INPUT_CHEAT_25, },
			{ { 'S'+1, 'T'+2, 'E'+3, 'R'+4, 'N'+5, 'O'+6, 'M'+7, 'A'+8, 'T'+9,						}, INPUT_CHEAT_26, },
			{ { 'F'+1, 'I'+2, 'R'+3, 'E'+4, 'H'+5, 'U'+6, 'R'+7, 'L'+8, 'E'+9, 'R'+10,				}, INPUT_CHEAT_27, },
			{ { 'C'+1, 'R'+2, 'O'+3, 'T'+4, 'C'+5, 'H'+6, 'B'+7, 'O'+8, 'M'+9, 'B'+10,				}, INPUT_CHEAT_28, },
	#ifdef SALES_DEMO
			{ { 'S'+1, 'E'+2, 'L'+3, 'L'+4, 																		}, INPUT_CHEAT_29, },	// Only used when SALES_DEMO defined
	#else
			{ { 'T'+1, 'H'+2, 'E'+3, 'R'+4, 'E'+5, 'S'+6, 'N'+7, 'O'+8, 'P'+9, 'L'+10, 'A'+11, 'C'+12, 'E'+13, 'L'+14, 'I'+15, 'K'+16,  'E'+17, 'O'+18, 'Z'+19, }, INPUT_CHEAT_29, },
	#endif
			{ { 'I'+1, 'A'+2, 'M'+3, 'S'+4, 'O'+5, 'L'+6, 'A'+7, 'M'+8, 'E'+9,						}, INPUT_CHEAT_30, },

#endif	// USE_LA_PALOMA_CHEATS
	};


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Set input mode.
//
// InputDemoInit() and InputDemoLoad() must be called before setting playback
// mode.
//
// InputDemoInit() must be called before setting record mode.
//
////////////////////////////////////////////////////////////////////////////////
extern void SetInputMode(
	INPUT_MODE mode)										// In:  Input mode
	{
	// Start new mode
	m_mode = mode;
	switch (m_mode)
		{
		case INPUT_MODE_LIVE:
			break;

		case INPUT_MODE_RECORD:
			// Reset index and number of entries
			m_lBufIndex = 0;
			m_lBufEntries = 0;
			break;

		case INPUT_MODE_PLAYBACK:
			// Reset index (retain whatever number of entries there currently are)
			m_lBufIndex = 0;
			break;

		default:
			// Change to known mode
			m_mode = INPUT_MODE_LIVE;
			TRACE("SetInputMode(): Unknown mode -- defaulting to INPUT_MODE_LIVE!\n");
			break;
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Get current input mode
//
////////////////////////////////////////////////////////////////////////////////
extern INPUT_MODE GetInputMode(void)				// Returns current mode
	{
	return m_mode;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Init demo mode.  Must be called before setting playback or record modes.
//
////////////////////////////////////////////////////////////////////////////////
extern short InputDemoInit(void)
	{
	short sResult = 0;

	// Reset index and number of entries
	m_lBufIndex = 0;
	m_lBufEntries = 0;

	// Allocate buffer
	if (m_pBuf == 0)
		{
		m_pBuf = new U32[BUF_MAX_ENTRIES];
		if (m_pBuf == 0)
			{
			sResult = -1;
			TRACE("InputDemoInit(): Error allocating buffer!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Kill demo mode.  Must be called if InputDemoInit() was successfull (safe
// to call even if it wasn't.)
//
////////////////////////////////////////////////////////////////////////////////
void InputDemoKill(void)
	{
	// Delete buffer
	delete []m_pBuf;
	m_pBuf = 0;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Load previously saved input demo data
//
////////////////////////////////////////////////////////////////////////////////
extern short InputDemoLoad(							// Returns 0 if successfull, non-zero otherwise
	RFile* pFile)											// In:  RFile to load from
	{
	short sResult = 0;

	ASSERT(m_pBuf);
	if (m_pBuf)
		{
		// Get number of entries
		if (pFile->Read(&m_lBufEntries) == 1)
			{
			if (m_lBufEntries <= BUF_MAX_ENTRIES)
				{

				// Load all entries
				for (long l = 0; l < m_lBufEntries; l++)
					pFile->Read(&m_pBuf[l]);
				
				// Check for errors
				if (pFile->Error())
					{
					sResult = -1;
					TRACE("InputDemoLoad(): Error reading data!\n");
					}
				}
			else
				{
				sResult = -1;
				TRACE("InputDemoLoad(): Too many entries to fit into current buffer size!\n");
				}
			}
		else
			{
			sResult = -1;
			TRACE("InputDemoLoad(): Error reading number of entries!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("InputDemoLoad(): No buffer!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Save current input demo data
//
////////////////////////////////////////////////////////////////////////////////
extern short InputDemoSave(							// Returns 0 if successfull, non-zero otherwise
	RFile* pFile)											// In:  RFile to save to
	{
	short sResult = 0;

	ASSERT(m_pBuf);
	if (m_pBuf)
		{
		// Save number of entries
		pFile->Write(m_lBufEntries);

		// Save all entries
		for (long l = 0; l < m_lBufEntries; l++)
			pFile->Write(m_pBuf[l]);

		// Check for errors
		if (pFile->Error())
			{
			sResult = -1;
			TRACE("InputDemoSave(): Error saving data!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("InputDemoSave(): No buffer!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Reset/Clear/Initialize local input.  This should be done just prior to 
// iteratively calling GetLocalInput().  It resets the last input storage and 
// positions the mouse, if mouse input is active.
//
////////////////////////////////////////////////////////////////////////////////
extern void ClearLocalInput(void)
	{
	// Reset last local input to nothing.
	ms_inputLastLocal	= INPUT_IDLE;

	// If mouse use is specified (be nice about background state) . . .
	if (g_InputSettings.m_sUseMouse != FALSE && rspIsBackground() == FALSE)
		{
		rspSetMouse(MOUSE_RESET_X, MOUSE_RESET_Y);
		}

	// Clear the keys' statuses.
	memset(rspGetKeyStatusArray(), 0, 128);

	// Clear cheats.
	short	i;
	for (i = 0; i < NUM_ELEMENTS(ms_acheats); i++)
		{
		ms_acheats[i].sCurrentIndex	= 0;
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Look for cheat combos.
//
////////////////////////////////////////////////////////////////////////////////
static void FindCheatCombos(	// Returns nothing.
	UINPUT*	pinput,				// In:  Input to augment.
										// Out: Input with cheats.
	RInputEvent* pie)				// In:  Latest input event or NULL.
	{
	long		lNow		= rspGetMilliseconds();
	short	i;

	if (pie)
		{
		if (pie->type == RInputEvent::Key)
			{
			long	lKey	= (pie->lKey & 0x00FF);
			// Force alpha lower keys to upper keys
			if (islower(lKey))
				lKey	= toupper(lKey);

			Cheat*	pcheat	= ms_acheats;
			for (i = 0; i < NUM_ELEMENTS(ms_acheats); i++, pcheat++)
				{
				// Been a while since last input?  We should/need to use rspGetMiliseconds() for
				// consistency.  This should offer no danger to synchronization as this is part
				// of the input structure which is transmitted to all machines in multiplayer and
				// loaded from disk in demos.
				if (pcheat->lLastValidInputTime + MAX_CONSEQ_CHEAT_KEY_LAG < lNow )
					{
					// Been too long -- Reset this cheat.
					pcheat->sCurrentIndex	= 0;
					}

				// Get current key.  Note that we detweak the strings so we 
				// can store the strings not as they are used to avoid these being
				// obvious when searching/viewing the exe.
				char	c = DETWEAK_CHAR(pcheat->szCheat, pcheat->sCurrentIndex);
				// If current key is hit . . .
				if ( lKey == (long)c && c != 0)
					{
					// Remember time of this key.
					pcheat->lLastValidInputTime				= lNow;
					// Advance index.
					(pcheat->sCurrentIndex)++;
					// Was that the last char . . . ?
					if (pcheat->szCheat[pcheat->sCurrentIndex] == '\0')
						{
						// Set input.
						(*pinput)	|= pcheat->input;
						// Reset.
						pcheat->sCurrentIndex	= 0;
						}
					}
				else
					{
					// Does not match -- Reset this cheat.
					pcheat->sCurrentIndex	= 0;
					}
				}
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// 
// Check to see if demo is over.  If this is not demo playback mode, then it
// will always return true.  If it is demo playback mode, then it will return
//	true if there is no more input.
//
////////////////////////////////////////////////////////////////////////////////

extern bool InputIsDemoOver(void)
{
	bool bOver = true;

	if (m_mode == INPUT_MODE_PLAYBACK && m_pBuf && m_lBufIndex < m_lBufEntries)
		bOver = false;

	return bOver;
}

// Simply returns TRUE or FALSE to say whether we should throw out a NextWeapon/PrevWeapon input or not.
bool CanCycleThroughWeapons()
{
#define WEAPON_SWITCH_HOLD_TIME 750
#define WEAPON_SWITCH_CYCLE_TIME 350
	static long lLastWeaponSwitchTime = 0;
	static bool bFastWeaponSwitching = false;
	long lCurTime = rspGetMilliseconds();
	bool bResult = false;

	if (lLastWeaponSwitchTime == 0)
		bResult = true;
	else if (bFastWeaponSwitching && lCurTime - lLastWeaponSwitchTime > WEAPON_SWITCH_HOLD_TIME)
	{
		bResult = true;
		bFastWeaponSwitching = false;
	}
	else if (bFastWeaponSwitching && lCurTime - lLastWeaponSwitchTime > WEAPON_SWITCH_CYCLE_TIME)
		bResult = true;
	else if (lCurTime - lLastWeaponSwitchTime > WEAPON_SWITCH_HOLD_TIME)
	{
		bResult = true;
		bFastWeaponSwitching = true;
	}
	else
		bResult = false;

	TRACE("CanCycleThroughWeapons Time %i %i %i Result %i", lCurTime, lLastWeaponSwitchTime, lCurTime - lLastWeaponSwitchTime, bResult);

	if (bResult)
		lLastWeaponSwitchTime = lCurTime;

	return bResult;
}

////////////////////////////////////////////////////////////////////////////////
//
// Get local input
//
////////////////////////////////////////////////////////////////////////////////
extern UINPUT GetLocalInput(				// Returns local input.
	CRealm* prealm,							// In:  Realm (used to access realm timer)
	RInputEvent* pie	/*= NULL*/)			// In:  Latest input event.  NULL to 
													//	disable cheats in a way that will be
													// harder to hack.
	{
	// Default to nothing
	UINPUT input = 0;

	if (m_mode == INPUT_MODE_PLAYBACK)
		{

		// Get input from buffer
		if (m_pBuf && (m_lBufIndex < m_lBufEntries))
			input = m_pBuf[m_lBufIndex++];

		}
	else
		{
// Set this to 0 to disable all possibility of any user input during the game
#if 1
		long				lCurTime			= prealm->m_time.GetGameTime();
		static long		lPrevTime		= lCurTime;
		// Get ptr to Blue's key status array.  Only need to do this
		// once.
		static U8*		pu8KeyStatus	= rspGetKeyStatusArray();

		short	sButtons	= 0;
		short	sDeltaX	= 360;

		// If utilizing mouse input . . .
		if (g_InputSettings.m_sUseMouse != FALSE && rspIsBackground() == FALSE)
			{
			short	sPosX, sPosY;
			short	sThreshY;
			rspGetMouse(&sPosX, &sPosY, &sButtons);
			rspSetMouse(MOUSE_RESET_X, MOUSE_RESET_Y);

			// Tweak input values.  We reduce the sensitivity by a factor of 3 to make
			// up for increased frame rates.
			double	dDeltaRot	= (MOUSE_RESET_X - sPosX) * (g_InputSettings.m_dMouseSensitivityX / 3.0);
#if 1
			// If positive round up . . .
			if (dDeltaRot >= 0.0)
				{
				dDeltaRot	+= 0.5;
				}
			else	// Negative round down.
				{
				dDeltaRot	-= 0.5;
				}
#endif
//			TRACE("sDif = %d, dDeltaRot = %g\n", (MOUSE_RESET_X - sPosX), dDeltaRot);

			// Must cast to short before subtracting b/c this statement is really:
			// sDeltaX = sDeltaX + dDeltaRot which became promoted to float before
			// it was added and then truncated causing a bias in degree toward
			// negative or rightward rotations.
			sDeltaX	+= (short)dDeltaRot;

			sThreshY	= MOUSE_Y_THRESH;
			if (g_InputSettings.m_dMouseSensitivityY > 0.0)
				{
				sThreshY	= short( float(sThreshY) / g_InputSettings.m_dMouseSensitivityY);
				}
			else
				{
				// Infiniti.
				sThreshY	*= 100;
				}

			// If less than last time . . .
			if (sDeltaX < 360)
				input	|= INPUT_RIGHT;
			else if (sDeltaX > 360)
				input |= INPUT_LEFT;

			if (sPosY < MOUSE_RESET_Y - sThreshY)
				input	|= INPUT_FORWARD;
			else if (sPosY > MOUSE_RESET_Y + sThreshY)
				input |= INPUT_BACKWARD;
			}

#if defined(ALLOW_JOYSTICK)
		U32	u32Buttons	= 0;

		// If utilizing joystick input . . .
		if (g_InputSettings.m_sUseJoy)
			{
			// Only need to update joystick 1.
			rspUpdateJoy(0);

			U32	u32Axes	= 0;
			rspGetJoyState(0, &u32Buttons, &u32Axes);	

#if defined(ALLOW_TWINSTICK)
			//if ((u32Axes & RSP_JOY_Y_NEG) || (u32Axes & RSP_JOY_Y_POS) || (u32Axes & RSP_JOY_X_NEG) || (u32Axes & RSP_JOY_X_POS))
				//input |= INPUT_FORWARD;
#else
			if (u32Axes & RSP_JOY_Y_NEG)
				{
				input	|= INPUT_FORWARD;
				}
			if (u32Axes & RSP_JOY_Y_POS)
				{
				input	|= INPUT_BACKWARD;
				}
			if (u32Axes & RSP_JOY_X_NEG)
				{
				input	|= INPUT_LEFT;
				}
			if (u32Axes & RSP_JOY_X_POS)
				{
				input	|= INPUT_RIGHT;
				}
			if (u32Axes & RSP_JOY_Z_NEG)
				{
				input	|= (INPUT_STRAFE | INPUT_LEFT);
				}
			if (u32Axes & RSP_JOY_Z_POS)
				{
				input	|= (INPUT_STRAFE | INPUT_RIGHT);
				}
#endif	// ALLOW_TWINSTICK
			}
#endif	// defined(ALLOW_JOYSTICK)

		if (IS_INPUT(CInputSettings::Forward))
			input |= INPUT_FORWARD;

		if (IS_INPUT(CInputSettings::Backward))
			input |= INPUT_BACKWARD;

		// The run key function is toggled by the caps lock key.
		// If the caps lock is on, run is walk.
		if ( (rspGetToggleKeyStates() & RSP_CAPS_LOCK_ON) == 0)
			{
			if (IS_INPUT(CInputSettings::Run) || IS_INPUT(CInputSettings::Run2) )
				input |= INPUT_RUN;
			}
		else
			{
			if ( (IS_INPUT(CInputSettings::Run) == 0) && (IS_INPUT(CInputSettings::Run2) == 0) )
				input |= INPUT_RUN;
			}

		// 2015 update: have the run button function as a walk button instead
		input ^= INPUT_RUN;

		if (IS_INPUT(CInputSettings::Strafe) || IS_INPUT(CInputSettings::Strafe2) )
			input |= INPUT_STRAFE;

		if (IS_INPUT(CInputSettings::StrafeLeft))
			input |= INPUT_STRAFE_LEFT;

		if (IS_INPUT(CInputSettings::StrafeRight))
			input |= INPUT_STRAFE_RIGHT;

		if (IS_INPUT(CInputSettings::Left))
			{
			input	|= INPUT_LEFT;
			}

		if (IS_INPUT(CInputSettings::Right))
			{
			input	|= INPUT_RIGHT;
			}

		if (input & INPUT_LEFT)
			{
			input |= INPUT_LEFT;
			// If last input had left rotation or this one has forward or reverse . . .
			if (	(ms_inputLastLocal & INPUT_LEFT) 
				||	(input & INPUT_FORWARD)
				||	(input & INPUT_BACKWARD) )
				{
				// Adjust rotation.
				double	dRate	= 0.0;	// Initialized for safety only.
				// If fast specified . . .
				switch (input & (INPUT_RUN | INPUT_FORWARD | INPUT_BACKWARD) )
					{
					case (INPUT_RUN | INPUT_FORWARD):						// Forward fast.
					case (INPUT_RUN | INPUT_BACKWARD):						// Backward fast.
					case (INPUT_RUN | INPUT_FORWARD | INPUT_BACKWARD):	// Some fucked up fast case.
						dRate	= g_InputSettings.m_dMovingFastDegreesPerSec;
						break;
					case INPUT_FORWARD:						// Forward normal.
					case INPUT_BACKWARD:						// Backward normal.
					case (INPUT_FORWARD | INPUT_BACKWARD):	// Some fucked up normal case.
						dRate	= g_InputSettings.m_dMovingSlowDegreesPerSec;
						break;
					case INPUT_RUN:							// Fast turn only.
						dRate	= g_InputSettings.m_dStillFastDegreesPerSec;
						break;
					case 0:										// Normal turn only.
						dRate	= g_InputSettings.m_dStillSlowDegreesPerSec;
						break;
					default:
						TRACE("GetLocalInput(): Unhandled input case.\n");
						break;
					}

				sDeltaX	+= ((lCurTime - lPrevTime) * dRate) / 1000UL;
                sDeltaX++;  // !!! FIXME: Not sure why this is needed. --ryan.
				}
			else
				{
				// Slight adjustment.
				sDeltaX	+= g_InputSettings.m_sTapRotationDegrees;
				}

			// Range check occurs later.
			}

		if (input & INPUT_RIGHT)
			{
			input |= INPUT_RIGHT;
			// If last input had right rotation or this one has forward or reverse . . .
			if (	(ms_inputLastLocal & INPUT_RIGHT) 
				||	(input & INPUT_FORWARD)
				||	(input & INPUT_BACKWARD) )
				{
				// Adjust rotation.
				double	dRate	= 0.0;	// Initialized for safety only.
				// If fast specified . . .
				switch (input & (INPUT_RUN | INPUT_FORWARD | INPUT_BACKWARD) )
					{
					case (INPUT_RUN | INPUT_FORWARD):						// Forward fast.
					case (INPUT_RUN | INPUT_BACKWARD):						// Backward fast.
					case (INPUT_RUN | INPUT_FORWARD | INPUT_BACKWARD):	// Some fucked up fast case.
						dRate	= g_InputSettings.m_dMovingFastDegreesPerSec;
						break;
					case INPUT_FORWARD:						// Forward normal.
					case INPUT_BACKWARD:						// Backward normal.
					case (INPUT_FORWARD | INPUT_BACKWARD):	// Some fucked up normal case.
						dRate	= g_InputSettings.m_dMovingSlowDegreesPerSec;
						break;
					case INPUT_RUN:							// Fast turn only.
						dRate	= g_InputSettings.m_dStillFastDegreesPerSec;
						break;
					case 0:										// Normal turn only.
						dRate	= g_InputSettings.m_dStillSlowDegreesPerSec;
						break;
					default:
						TRACE("GetLocalInput(): Unhandled input case.\n");
						break;
					}

				sDeltaX	-= ((lCurTime - lPrevTime) * dRate) / 1000UL;
				}
			else
				{
				// Slight adjustment.
				sDeltaX	-= g_InputSettings.m_sTapRotationDegrees;
				}

			// Range check occurs later.
			}

		if (IS_INPUT(CInputSettings::Fire) || IS_INPUT(CInputSettings::Fire2) )
			{
			input |= INPUT_FIRE;
			}

		if (IS_INPUT(CInputSettings::Duck))
			{
			input	|= INPUT_DUCK;
			}

		if (IS_INPUT(CInputSettings::Up))
			{
			input	|= INPUT_MOVE_UP;
			}
		if (IS_INPUT(CInputSettings::Down))
			{
			input	|= INPUT_MOVE_DOWN;
			}
		if (IS_INPUT(CInputSettings::MoveLeft))
			{
			input	|= INPUT_MOVE_LEFT;
			}
		if (IS_INPUT(CInputSettings::MoveRight))
			{
			input	|= INPUT_MOVE_RIGHT;
			}
		if (IS_INPUT(CInputSettings::FireUp))
		{
			input |= INPUT_FIRE_UP;
		}
		if (IS_INPUT(CInputSettings::FireDown))
		{
			input |= INPUT_FIRE_DOWN;
		}
		if (IS_INPUT(CInputSettings::FireLeft))
		{
			input |= INPUT_FIRE_LEFT;
		}
		if (IS_INPUT(CInputSettings::FireRight))
		{
			input |= INPUT_FIRE_RIGHT;
		}

		if (WAS_INPUT(CInputSettings::Revive))
			{
			input |= INPUT_REVIVE;
			CLEAR_INPUT(CInputSettings::Revive);
			}

		// Find deluxe cheat combos.
		FindCheatCombos(&input, pie);

		if (WAS_INPUT(CInputSettings::Weapon0))
			{
			input |= INPUT_WEAPON_0;
			CLEAR_INPUT(CInputSettings::Weapon0);
			}
		else if (WAS_INPUT(CInputSettings::Weapon1))
			{
			input |= INPUT_WEAPON_1;
			CLEAR_INPUT(CInputSettings::Weapon1);
			}
		else if (WAS_INPUT(CInputSettings::Weapon2))
			{
			input |= INPUT_WEAPON_2;
			CLEAR_INPUT(CInputSettings::Weapon2);
			}
		else if (WAS_INPUT(CInputSettings::Weapon3))
			{
			input |= INPUT_WEAPON_3;
			CLEAR_INPUT(CInputSettings::Weapon3);
			}
		else if (WAS_INPUT(CInputSettings::Weapon4))
			{
			input |= INPUT_WEAPON_4;
			CLEAR_INPUT(CInputSettings::Weapon4);
			}
		else if (WAS_INPUT(CInputSettings::Weapon5))
			{
			input |= INPUT_WEAPON_5;
			CLEAR_INPUT(CInputSettings::Weapon5);
			}
		else if (WAS_INPUT(CInputSettings::Weapon6))
			{
			input |= INPUT_WEAPON_6;
			CLEAR_INPUT(CInputSettings::Weapon6);
			}
		else if (WAS_INPUT(CInputSettings::Weapon7))
			{
			input |= INPUT_WEAPON_7;
			CLEAR_INPUT(CInputSettings::Weapon7);
			}
		else if (WAS_INPUT(CInputSettings::Weapon8))
			{
			input |= INPUT_WEAPON_8;
			CLEAR_INPUT(CInputSettings::Weapon8);
			}
		else if (WAS_INPUT(CInputSettings::Weapon9))
			{
			input |= INPUT_WEAPON_9;
			CLEAR_INPUT(CInputSettings::Weapon9);
			}
		else if (WAS_INPUT(CInputSettings::Weapon10))
			{
			input |= INPUT_WEAPON_10;
			CLEAR_INPUT(CInputSettings::Weapon10);
			}
		else if (WAS_INPUT(CInputSettings::NextWeapon))
			{
			if (CanCycleThroughWeapons())
				input |= INPUT_WEAPON_NEXT;
			CLEAR_INPUT(CInputSettings::NextWeapon);
			}
		else if (WAS_INPUT(CInputSettings::PrevWeapon))
			{
			if (CanCycleThroughWeapons())
				input |= INPUT_WEAPON_PREV;
			CLEAR_INPUT(CInputSettings::PrevWeapon);
			}

		if (WAS_INPUT(CInputSettings::Execute))
			{
// It is not proper to execute people in non-violent countries.
#if VIOLENT_LOCALE
			input |= INPUT_EXECUTE;
#endif
			CLEAR_INPUT(CInputSettings::Execute);
			}

		if (WAS_INPUT(CInputSettings::Suicide))
			{
			input |= INPUT_SUICIDE;
			CLEAR_INPUT(CInputSettings::Suicide);
			}

		if (IS_INPUT(CInputSettings::NextMap))
		{
			prealm->m_bPressedEndLevelKey = true;
		}

		// Keep in range.
		if (sDeltaX > 720)
			sDeltaX	= 720;
		else if (sDeltaX < 0)
			sDeltaX	= 0;

		// Set only the 10 bits of the delta (as unsigned value).
		// As long as the above range check is used, we don't need
		// to & with the rotation mask.
		input		|=	(U32)sDeltaX;

#ifdef MOBILE
		input = AndroidGetInput();
#endif

		// Store time for next call.
		lPrevTime	= lCurTime;

		// Store input for next call.
		ms_inputLastLocal	= input;

		// If in record mode, save input to buffer
		if (m_mode == INPUT_MODE_RECORD)
			{
			if (m_pBuf && (m_lBufIndex < BUF_MAX_ENTRIES))
				{
				// If this is the last entry that will fit, then commit suicide
				if (m_lBufIndex == (BUF_MAX_ENTRIES - 1))
					input |= INPUT_SUICIDE;

				m_pBuf[m_lBufIndex++] = input;
				m_lBufEntries++;
				}
			else
				{
				}
			}
#endif
		}

	return input;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
