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
// input.h
// Project: Postal
//
// History:
//		12/05/96 MJR	Started.
//
//		02/19/97 BRH	Modified to include the keys we are currently using
//							for CDude control in the game.  Added functions for
//							getting and setting input for local and remote play.
//
//		03/03/97	JMI	Added macros for keys 1 through 5 (CDude will use
//							these to chose a weapon).
//
//		03/14/97	JMI	Added macros for space key.
//
//		03/24/97	JMI	Changed typedef for UINPUT from U16 to U32.
//							Added INPUT_ROT_MASK.  The first 9 bits are now reserved
//							for a rotation value between 0 and 359 inclusive.  This
//							pushed all the other bits up a bit, but I don't think that
//							anything currently uses this API w/o the INPUT_* macros.
//
//		03/26/97	JMI	Was getting rotation for mouse input backwards.  Fixed.
//
//		03/27/97	JMI	Added macros for numpad 0 key (CDude will use this for
//							duck).
//
//		03/27/97	JMI	Shortcutted INSERT to same as numpad 0 as patch for now.
//
//		03/31/97	JMI	Now the actual keys used are chosen by g_InputSettings.
//							Therefore, macros are now more generic (e.g., INPUT_R
//							became INPUT_RIGHT, and INPUT_ALT became INPUT_FIRE).
//							Added g_InputSettings declaration.
//
//		04/01/97	JMI	Now we used 10 bits for the rotation value and, although
//							it is stored as unsigned (0..720), it represents a postive
//							or negative delta between -360 and 360.
//
//		04/03/97	JMI	Now checks rspIsBackground() before doing mouse stuff.
//
//		04/11/97 MJR	Moved lots of stuff into input.cpp and added support for
//							recording and playing back inputs for self-playing mode.
//
//		04/11/97	JMI	Changed INPUT_MENU to INPUT_SUICIDE.
//							Also, added INPUT_NEXT_LEVEL.
//							Added an RInputEvent* to GetLocalInput() proto.
//							externs for m_aInputs were not specifying a type and, there-
//							fore, it was an 'int' array instead of UINPUT.
//
//		04/22/97	JMI	Added extern of g_InputSettings.
//
//		04/24/97	JMI	Moved around INPUT_JUMP, INPUT_SUICIDE, and 
//							INPUT_NEXT_LEVEL.
//							Also, added INPUT_WEAPON_6...9 and INPUT_EXECUTE.  
//							Consequently, we're out of inputs (and we still don't have
//							INPUT_TOGGLE_HUMANSHIELD).
//
//		05/02/97	JMI	Increased number of available input bits by reducing
//							the weapon changing bits to 4 bits (now a number instead of
//							bit fields).
//
//		05/13/97	JMI	Changed INPUT_RUN to INPUT_WALK.
//
//		05/14/97	JMI	Added INPUT_PICKUP.
//
//		06/09/97	JMI	Changed INPUT_WALK to INPUT_RUN.
//
//		06/15/97 MJR	Removed INPUT_NEXT_LEVEL, which has now been replaced by
//							an client/server mechanism.
//
//		06/15/97 MJR	Changed to separate InputDemoInit() and InputDemoKill()
//							functions to make it real obvious and clean.
//
//		07/15/97	JMI	Added INPUT_WEAPONS_BIT.
//
//		07/19/97	JMI	Added InitLocalInput().
//
//		07/25/97	JMI	Added more INPUT_*'s and changed INPUT_WEAPON_11,12,13,14
//							to INPUT_CHEAT_11,12,13,14.  There are now 20 possible
//							cheat inputs.
//							Also, removed INPUT_PICKUP.
//
//		08/06/97	JMI	Changed InitLocalInput() to ClearLocalInput().
//
//		08/10/97	JMI	Added StrafeLeft and StrafeRight inputs.
//							Also, changed Jump to Revive and INPUT_JUMP to INPUT_REVIVE.
//							Had put INPUT_JUMP back temporarily b/c I want to check in
//							this and all the files that depend on the new INPUT_REVIVE 
//							in but play.cpp has been checked out for a long time now.
//
//		08/12/97	JMI	Now requires an input event for cheats for GetLocalInput().
//
//		08/17/97	JMI	Got rid of INPUT_JUMP, now play.cpp uses INPUT_REVIVE.
//
//		08/26/97 BRH	Added query function InputIsDemoOver() for the special
//							case ending demo that will be played when the player wins,
//
//		09/06/97 MJR	Added INPUT_IDLE.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INPUT_H
#define INPUT_H

#include "RSPiX.h"
#include "realm.h"
#include "InputSettings.h"


// Maximum number of dudes supported by this module.  Just about any value
// will work, it merely uses up a few extra bytes per dude, and no extra
// processing.  This is currently set to a value likely to be far beyond any
// the number of dudes that the main app could support.
#define INPUT_MAX_DUDES	32


// Macros to access bits within an UINPUT type.  There are two types of macros:
// "masks" and "bit numbers".  The masks are designed to set or get the value
// of a bit.  The bit numbers are designed to make it easier to shift the bits
// around.  Note that the bit numbers go from 0 to 'n' (currently 15).
//
// The order of the bits should not be changed without carefully weighing the
// consequences.  It is highly likely that some code will end up relying on the
// order of the bits for efficiency sake, so changing the order of the bits
// could break existing code.  Any such code should certainly refer to these
// macros in some way, so that if it becomes absolutely necessary to change
// this stuff, we can at least track down all the code that uses it.

// The first 10 bits are reserved for a rotation delta value between -359 and 359.
#define INPUT_ROT_MASK		0x000003FF

#define INPUT_REVIVE			0x00000400
#define INPUT_REVIVE_BIT	10	    
									    
#define INPUT_SUICIDE		0x00000800
#define INPUT_SUICIDE_BIT	11

#define INPUT_RIGHT			0x00001000
#define INPUT_RIGHT_BIT		12	    
#define INPUT_LEFT			0x00002000
#define INPUT_LEFT_BIT		13	    
#define INPUT_BACKWARD		0x00004000
#define INPUT_BACKWARD_BIT	14	    
#define INPUT_FORWARD		0x00008000
#define INPUT_FORWARD_BIT	15    
#define INPUT_DIR_MASK		0x0000f000
#define INPUT_STRAFE			0x00010000
#define INPUT_STRAFE_BIT	16    
#define INPUT_FIRE			0x00020000
#define INPUT_FIRE_BIT		17    
#define INPUT_RUN				0x00040000
#define INPUT_RUN_BIT		18    
#define INPUT_DUCK			0x00080000
#define INPUT_DUCK_BIT		19
									    
#define INPUT_WEAPON_0		0x00100000
#define INPUT_WEAPON_1		0x00200000
#define INPUT_WEAPON_2		0x00300000
#define INPUT_WEAPON_3		0x00400000
#define INPUT_WEAPON_4		0x00500000
#define INPUT_WEAPON_5		0x00600000
#define INPUT_WEAPON_6		0x00700000
#define INPUT_WEAPON_7		0x00800000
#define INPUT_WEAPON_8		0x00900000
#define INPUT_WEAPON_9		0x00A00000
#define INPUT_WEAPON_10		0x00B00000
#define INPUT_CHEAT_11		0x00C00000
#define INPUT_CHEAT_12		0x00D00000
#define INPUT_CHEAT_13		0x00E00000
#define INPUT_CHEAT_14		0x00F00000
#define INPUT_CHEAT_15		0x01000000
#define INPUT_CHEAT_16		0x01100000
#define INPUT_CHEAT_17		0x01200000
#define INPUT_CHEAT_18		0x01300000
#define INPUT_CHEAT_19		0x01400000
#define INPUT_CHEAT_20		0x01500000
#define INPUT_CHEAT_21		0x01600000
#define INPUT_CHEAT_22		0x01700000
#define INPUT_CHEAT_23		0x01800000
#define INPUT_CHEAT_24		0x01900000
#define INPUT_CHEAT_25		0x01A00000
#define INPUT_CHEAT_26		0x01B00000
#define INPUT_CHEAT_27		0x01C00000
#define INPUT_CHEAT_28		0x01D00000
#define INPUT_CHEAT_29		0x01E00000
#define INPUT_CHEAT_30		0x01F00000

#define INPUT_WEAPONS_MASK	0x01F00000	// Includes cheats!
#define INPUT_WEAPONS_BIT	20	// thru 24.

//#ifdef MOBILE
#define INPUT_WEAPON_NEXT		0x02000000
#define INPUT_WEAPON_PREV		0x04000000
#define INPUT_ROT_IS_ABS		0x08000000
//#endif

#define INPUT_EXECUTE		0x40000000
#define INPUT_EXECUTE_BIT	30

// Extra flags for keyboard "twinstick" inputs. We could have done this with the ROT_IS_ABS for moving OR shooting
// but then we'd be stuck on the other.
#define INPUT_MOVE_UP		0x00100000000LL
#define INPUT_MOVE_DOWN		0x00200000000LL
#define INPUT_MOVE_LEFT		0x00400000000LL
#define INPUT_MOVE_RIGHT	0x00800000000LL
#define INPUT_FIRE_UP		0x01000000000LL
#define INPUT_FIRE_DOWN		0x02000000000LL
#define INPUT_FIRE_LEFT		0x04000000000LL
#define INPUT_FIRE_RIGHT	0x08000000000LL
#define INPUT_STRAFE_LEFT	0x10000000000LL
#define INPUT_STRAFE_RIGHT	0x20000000000LL

// Use this for when you don't want the dude to do anything
#define INPUT_IDLE			360

// Available input modes
typedef enum
	{
	INPUT_MODE_LIVE,
	INPUT_MODE_RECORD,
	INPUT_MODE_PLAYBACK
	} INPUT_MODE;

// UINPUT type
// Had to rename this from INPUT because of a pre-existing type
// Redefined to U64 because we ran out of input room.
typedef U64 UINPUT;

// Global input settings.
extern CInputSettings g_InputSettings;


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
	INPUT_MODE mode);										// In:  Input mode


////////////////////////////////////////////////////////////////////////////////
//
// Get current input mode
//
////////////////////////////////////////////////////////////////////////////////
extern INPUT_MODE GetInputMode(void);				// Returns current mode


////////////////////////////////////////////////////////////////////////////////
//
// Init demo mode.  Must be called before setting playback or record modes.
//
////////////////////////////////////////////////////////////////////////////////
extern short InputDemoInit(void);					// Returns 0 if successfull, non-zero otherwise


////////////////////////////////////////////////////////////////////////////////
//
// Kill demo mode.  Must be called if InputDemoInit() was successfull (safe
// to call even if it wasn't.)
//
////////////////////////////////////////////////////////////////////////////////
extern void InputDemoKill(void);


////////////////////////////////////////////////////////////////////////////////
//
// Load previously saved input demo data
//
////////////////////////////////////////////////////////////////////////////////
extern short InputDemoLoad(							// Returns 0 if successfull, non-zero otherwise
	RFile* pFile);											// In:  RFile to load from


////////////////////////////////////////////////////////////////////////////////
//
// Save current input demo data
//
////////////////////////////////////////////////////////////////////////////////
extern short InputDemoSave(							// Returns 0 if successfull, non-zero otherwise
	RFile* pFile);											// In:  RFile to save to


////////////////////////////////////////////////////////////////////////////////
//
// Reset/Clear/Initialize local input.  This should be done just prior to 
// iteratively calling GetLocalInput().  It resets the last input storage and 
// positions the mouse, if mouse input is active.
//
////////////////////////////////////////////////////////////////////////////////
extern void ClearLocalInput(void);

////////////////////////////////////////////////////////////////////////////////
//
// InputIsDemoOver - query function that returns true if there is no more
//							demo data.  It will always return true if we are not in
//							demo playback mode.
//
////////////////////////////////////////////////////////////////////////////////

extern bool InputIsDemoOver(void);		// Returns true when demo is over

////////////////////////////////////////////////////////////////////////////////
//
// Get local input
//
////////////////////////////////////////////////////////////////////////////////
extern UINPUT GetLocalInput(				// Returns local input structure.
	CRealm* prealm,							// In:  Realm (used to access realm timer)
	RInputEvent* pie	= NULL);				// In:  Latest input event.  NULL to 
													//	disable cheats in a way that will be
													// harder to hack.


////////////////////////////////////////////////////////////////////////////////
//
// Get input for specified dude
//
////////////////////////////////////////////////////////////////////////////////
inline UINPUT GetInput(short sDudeNumber)
	{
	ASSERT(sDudeNumber < INPUT_MAX_DUDES);
	extern UINPUT m_aInputs[INPUT_MAX_DUDES];
	return m_aInputs[sDudeNumber];
	}


////////////////////////////////////////////////////////////////////////////////
//
// Set input for specified dude
//
////////////////////////////////////////////////////////////////////////////////
inline void SetInput(short sDudeNumber, UINPUT input)
	{
	ASSERT(sDudeNumber < INPUT_MAX_DUDES);
	extern UINPUT m_aInputs[INPUT_MAX_DUDES];
	m_aInputs[sDudeNumber] = input;
	}


#endif //INPUT_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
