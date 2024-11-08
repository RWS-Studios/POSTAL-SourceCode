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
////////////////////////////////////////////////////////////////////////////////
//
// macblue.h
//
// This header defines the Unix implementation of the RSPiX BLUE layer.
//
// History:
//		06/01/04 RCG    Added.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef UNIXBLUE_H
#define UNIXBLUE_H

#include "UnixSystem.h"
#include "UnixBlueKeys.h"

////////////////////////////////////////////////////////////////////////////////
// Blue API
////////////////////////////////////////////////////////////////////////////////

extern int16_t rspInitBlue(void);

extern void rspKillBlue(void);

#define RSP_DOSYSTEM_HOGCPU		0
#define RSP_DOSYSTEM_TOLERATEOS	1
#define RSP_DOSYSTEM_SLEEP			2

extern void rspSetDoSystemMode(
	int16_t sMode);									// In:  Mode (use RSP_DOSYSTEM_* macros)

extern void rspDoSystem(void);

// Global variable that lets app set the number of blocks that it guesses it
// might allocate from th heap.  This may help prevent fragmentation.  We set
// the default value, and it's up to the app to change it before calling
// rspInitBlue() if it wants to make a better guess.
extern int16_t macGuessTotalHeapBlocks;

// Global variable that lets app set the amount of memory (in bytes) to reserve.
// Once the app has exhausted all other memory, a warning will be displayed
// saying that memory is very low.  The reserved memory is then freed, and the
// application is allowed to continue.  If the application exhausts all memory
// again, an error dialog is displayed and the application is terminated.
// NOTE: Currently used only with SmartHeap -- see mmain.cpp for details.
extern int32_t macReserveMemBytes;


////////////////////////////////////////////////////////////////////////////////
// Debug API
//
// Define the TRACE, STRACE, and ASSERT macros.  If _DEBUG or TRACENASSERT are
// defined, then these macros are usefull debugging aids.  Otherwise, it is
// assumed that the program is being compiled in "release" mode, and all three
// of the macros are changed such that no code nor data results from their
// use, thereby eliminating all traces of them from the program.
//
// TRACE is like printf, but sends the output to the debug window.  Note that
// it slips in the file and line number information before printing whatever
// the user requested.
//
// STRACE is like TRACE, except that it doesn't display the file and line
// number information.
//
// ASSERT is used to assert that an expression is true.  If it is, in fact,
// true, then ASSERT does nothing.  If not, then it calls rspAssert().  See
// that function decleration for more details.
//
////////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) || defined(TRACENASSERT)
	// TRACE macro, the preferred method of sending output to debug window
	#define STRACE			rspTrace
	#define TRACE			STRACE("%s(%d):", __FILE__, __LINE__),STRACE

	// ASSERT macro, the preferred method of asserting that expressions are true.
	//#define ASSERT(a)		while (!(a))  \
	//								if (rspAssert(__FILE__, __LINE__, #a) == 0)  \
	//									break
    #define ASSERT SDL_assert
#else
	// This causes the compiler to "optimize" out the data and the function call.
	// We can't simply define TRACE as nothing (like ASSERT) because it takes
	// a variable number of arguments.  So, this is the next best thing.
	#define STRACE			1 ? (void)0 : rspTrace
	#define TRACE			STRACE

	// Simply remove the function and it's paramters.
	#define ASSERT(a)
#endif

// Trace works like printf, but sends output to debug window.  This is rarely
// called directly.  Instead, use the TRACE() macro so that the TRACE is
// automatically removed in "release" versions of the program.
extern void rspTrace(char* szFrmt, ...);

// Assert checks the expression and, if it is zero, displays an alert box
// and waits for the user to select ABORT, RETRY or IGNORE.  This is rarely
// called directly.  Instead, use the ASSERT() macro so that the ASSERT is
// automatically removed in "release" versions of the program.
extern int16_t rspAssert(	// Returns result.
	char* pszFile,			// Source file.
	int16_t sLine,			// Source line.
	char* pszExpr);		// String representing expression.


////////////////////////////////////////////////////////////////////////////////
// Mouse API
////////////////////////////////////////////////////////////////////////////////

// Macros for GetMouseEvent, GetLastMouseEvent.
#define RSP_MB_NONE				0	// No mouse event
#define RSP_MB0_RELEASED		1	// Mouse button 0 was released
#define RSP_MB0_PRESSED			2	// Mouse button 0 was pressed
#define RSP_MB0_DOUBLECLICK	3	// Mouse button 0 was double clicked
#define RSP_MB1_RELEASED		4	// Defined for compile-campatibility with PC!
#define RSP_MB1_PRESSED			5	// Defined for compile-campatibility with PC!
#define RSP_MB1_DOUBLECLICK	6	// Defined for compile-campatibility with PC!
#define RSP_MB2_RELEASED		7	// Defined for compile-campatibility with PC!
#define RSP_MB2_PRESSED			8	// Defined for compile-campatibility with PC!
#define RSP_MB2_DOUBLECLICK	9	// Defined for compile-campatibility with PC!

extern void rspGetMouse(
	int16_t* psX,				// X position returned here (unless NULL)
	int16_t* psY,				// Y position returned here (unless NULL)
	int16_t* psButtons);	// button status returned here (unless NULL)

extern void rspSetMouse(
	int16_t sX,				// New x position
	int16_t sY);				// New y position

extern int16_t rspGetMouseEvent(	// Returns 0 if no event was available, non-zero otherwise
	int16_t* psX,							// Event's X position is returned here (unless NULL)
	int16_t* psY,							// Event's Y position is returned here (unless NULL)
	int16_t* psButton,					// Event's button status is returned here (unless NULL)
	int32_t* plTime = NULL,					// Event's time stamp returned here (unless NULL)
	int16_t* psType = NULL);			// Event's type (as per OS) is returned here (unless NULL)

extern int16_t rspGetLastMouseEvent(	// Returns 0 if no event was available, non-zero otherwise
	int16_t* psX,								// Event's X position is returned here (unless NULL)
	int16_t* psY,								// Event's Y position is returned here (unless NULL)
	int16_t* psButton,						// Event's button status is returned here (unless NULL)
	int32_t* plTime = NULL,					// Event's time stamp returned here (unless NULL)
	int16_t* psType = NULL);				// Event's type (as per OS) is returned here (unless NULL)

extern void rspClearMouseEvents(void);

extern void rspHideMouseCursor(void);

extern void rspShowMouseCursor(void);

// Get the current cursor show level.  A level of 1 or greater means the cursor
// is currently showing, while a level of 0 or less means the cursor is hidden.
extern int16_t rspGetMouseCursorShowLevel(void);

// Set the current cursor show level.  A level of 1 or greater means the cursor
// is currently showing, while a level of 0 or less means the cursor is hidden.
extern void rspSetMouseCursorShowLevel(
	int16_t sLevel);												// In:  New cursor level

// Global variables for setting maximum mouse movement between two events,
// beyond which they would no longer be considered double-clicks.  These are
// set by this module to reasonable values, but an application CAN change
// them.  This is, however, a MAC-SPECIFIC EXTENSION, so user beware!
extern int16_t mMouseDoubleClickX;
extern int16_t mMouseDoubleClickY;


////////////////////////////////////////////////////////////////////////////////
// Keyboard API
////////////////////////////////////////////////////////////////////////////////

extern void rspScanKeys(
	uint8_t* pucKeys);				// Out: Array of 128 unsigned chars (one per SK code)

extern void rspClearKeyEvents(void);

extern int16_t rspGetKey(						// Returns 1 if key was available, 0 if not
	int32_t* plKey,								// Out: Key info (0 if no key was available)
	int32_t* plTime = NULL);					// Out: Key's time stamp (unless NULL)

extern int16_t rspIsKey(void);				// Returns 1 if key is available, 0 if not

// This function returns a pointer to an array of 128 bytes.  Each byte indexed
// by an RSP_SK_* macro indicates the status of that key.  If any element in
// the array is 0 when the corresponding key is pressed, that key is set to 1.
// When that key is released, it is incremented to 2.  When it is pressed again,
// it is incremented to 3, etc., etc..  This array is only cleared by the caller
// for maximum flexibility.  Note that, if the array is cleared, and a key is
// released the entry put into the array will be 2 (not 1) so that the caller
// can rely upon the meaning of evens vs. odds (key currently down vs. up).
// Also, note that this array is static and, therefore, this function need NOT
// be called for every use of the array.  As a matter of fact, you may only need
// to call this function once for an entire program's execution of scans and 
// clears of the array.
extern U8* rspGetKeyStatusArray(void);	// Returns pointer to 128-byte key status array


// Return the state of the toggle keys (caps lock, num lock, and scroll lock.)
//
// Each such key that is currently "on" has the corresponding bit set to 1 in
// the returned value.
//
// Note that the values returned by this function are not guaranteed to be in
// synchronization with any of the other key functions.  Their state is obtained
// as close to the actual key status as is possible.
extern int32_t rspGetToggleKeyStates(void);	// Returns toggle key state flags

// Flags used to test value returned by rspGetToggleKeyStates()
#define RSP_CAPS_LOCK_ON			0x00000001
#define RSP_NUM_LOCK_ON				0x00000002
#define RSP_SCROLL_LOCK_ON			0x00000004


///////////////////////////////////////////////////////////////////////////////
// JOYSTICK Macros:

// Joystick identifiers.
// Use 0 to specify first joystick and 1 to specify second.

// Joy button flags for pu32Buttons parameter to rspGetJoyState() and
// rspGetJoyPrevState().
#define RSP_JOY_BUT_1			0x00000001
#define RSP_JOY_BUT_2			0x00000002
#define RSP_JOY_BUT_3			0x00000004
#define RSP_JOY_BUT_4			0x00000008
#define RSP_JOY_BUT_5			0x00000010
#define RSP_JOY_BUT_6			0x00000020 
#define RSP_JOY_BUT_7			0x00000040
#define RSP_JOY_BUT_8			0x00000080
#define RSP_JOY_BUT_9			0x00000100
#define RSP_JOY_BUT_10			0x00000200 
#define RSP_JOY_BUT_11			0x00000400
#define RSP_JOY_BUT_12			0x00000800
#define RSP_JOY_BUT_13			0x00001000
#define RSP_JOY_BUT_14			0x00002000 
#define RSP_JOY_BUT_15			0x00004000
#define RSP_JOY_BUT_16			0x00008000
#define RSP_JOY_BUT_17			0x00010000
#define RSP_JOY_BUT_18			0x00020000
#define RSP_JOY_BUT_19			0x00040000
#define RSP_JOY_BUT_20			0x00080000
#define RSP_JOY_BUT_21			0x00100000
#define RSP_JOY_BUT_22			0x00200000 
#define RSP_JOY_BUT_23			0x00400000
#define RSP_JOY_BUT_24			0x00800000
#define RSP_JOY_BUT_25			0x01000000
#define RSP_JOY_BUT_26			0x02000000 
#define RSP_JOY_BUT_27			0x04000000
#define RSP_JOY_BUT_28			0x08000000
#define RSP_JOY_BUT_29			0x10000000
#define RSP_JOY_BUT_30			0x20000000 
#define RSP_JOY_BUT_31			0x40000000
#define RSP_JOY_BUT_32			0x80000000

// Joy dir flags for pu32Axes parameter to rspGetJoyState() and
// rspGetJoyPrevState().
#define RSP_JOY_X_POS			0x00000001
#define RSP_JOY_X_NEG			0x00000002
#define RSP_JOY_Y_POS			0x00000004
#define RSP_JOY_Y_NEG			0x00000008
#define RSP_JOY_Z_POS			0x00000010
#define RSP_JOY_Z_NEG			0x00000020
#define RSP_JOY_W_POS			0x00000040
#define RSP_JOY_W_NEG			0x00000080
#define RSP_JOY_U_POS			0x00000100
#define RSP_JOY_U_NEG			0x00000200
#define RSP_JOY_V_POS			0x00000400
#define RSP_JOY_V_NEG			0x00000800


///////////////////////////////////////////////////////////////////////////////
// JOYSTICK Typedefs.

// Represents the positions for all axes of a particular joystick.
// Use this as the pjoypos parameter to rspGetJoyPos() and rspGetJoyPrevPos().
typedef struct
	{
	int32_t	lX;
	int32_t	lY;
	int32_t	lZ;
	int32_t	lW;
	int32_t	lU;
	int32_t	lV;
	} RJoyPos;

///////////////////////////////////////////////////////////////////////////////
// JOYSTICK Prototypes.

// Updates joystick sJoy's current state and makes the current state the 
// previous.
extern void rspUpdateJoy(
		int16_t sJoy);	// In:  Joystick to query.

// Puts the coordinates of joystick sJoy's position in your longs.
// This function returns directions in an analog format (0..0xFFFF).
extern void rspGetJoyPos(
	int16_t sJoy,					// In:  Joystick to query.
	int32_t *plX,					// Out: X axis position of joystick, if not NULL.
	int32_t *plY = NULL,			// Out: Y axis position of joystick, if not NULL.
	int32_t *plZ = NULL,			// Out: Z axis position of joystick, if not NULL.
	int32_t *plW = NULL,			// Out: W axis position of joystick, if not NULL.
	int32_t *plU = NULL,			// Out: U axis position of joystick, if not NULL.
	int32_t *plV = NULL);		// Out: V axis position of joystick, if not NULL.

extern void rspGetJoyPos(
	int16_t sJoy,					// In:  Joystick to query.
	RJoyPos* pjoypos);		// In:  Joystick positions for all axes.

// Puts the coordinates of the previous joystick sJoy's position in your longs.
// This function returns directions in an analog format (0..0xFFFF).
extern void rspGetJoyPrevPos(
	int16_t sJoy,					// In:  Joystick to query.
	int32_t *plX,					// Out: X axis position of joystick, if not NULL.
	int32_t *plY = NULL,			// Out: Y axis position of joystick, if not NULL.
	int32_t *plZ = NULL,			// Out: Z axis position of joystick, if not NULL.
	int32_t *plW = NULL,			// Out: W axis position of joystick, if not NULL.
	int32_t *plU = NULL,			// Out: U axis position of joystick, if not NULL.
	int32_t *plV = NULL);		// Out: V axis position of joystick, if not NULL.

extern void rspGetJoyPrevPos(
	int16_t sJoy,					// In:  Joystick to query.
	RJoyPos* pjoypos);		// In:  Joystick positions for all axes.

// Reads the joystick sJoy's current state.
// This function returns directions in a digital format (up, down, centered).
extern void rspGetJoyState(
	int16_t sJoy,					// In:  Joystick to query.
	U32*	pu32Buttons,		// Out: Buttons that are down, if not NULL.
									// An RSP_JOY_BUT_## bit field that is set indicates
									// that button is down.
	U32*	pu32Axes = NULL);	// Out: Directions that are specificed, if not NULL.
									// An RSP_JOY_?_POS bit set indicates the ? axis is positive.
									// An RSP_JOY_?_NEG bit set indicates the ? axis is negative.
									// If neither is set for ? axis, that axis is 0.

// Reads the joystick sJoy's previous state.
// This function returns directions in a digital format (up, down, centered).
extern void rspGetJoyPrevState(
	int16_t sJoy,					// In:  Joystick to query.
	U32*	pu32Buttons,		// Out: Buttons that are down, if not NULL.
									// An RSP_JOY_BUT_## bit field that is set indicates
									// that button is down.
	U32*	pu32Axes = NULL);	// Out: Directions that are specificed, if not NULL.
									// An RSP_JOY_?_POS bit set indicates the ? axis is positive.
									// An RSP_JOY_?_NEG bit set indicates the ? axis is negative.
									// If neither is set for ? axis, that axis is 0.


extern bool GetDudeFireAngle(double* d_Angle);
extern void GetDudeVelocity(double* d_Velocity, double* d_Angle);

// Functions to convert bitfields to joybutton numbers and back again.
extern int16_t JoyBitfieldToIndex(U32 bitfield);
extern U32 JoyIndexToBitfield(int16_t index);
extern int16_t MouseBitfieldToIndex(U32 bitfield);
extern U32 MouseIndexToBitfield(int16_t index);

////////////////////////////////////////////////////////////////////////////////
// DISPLAY API
////////////////////////////////////////////////////////////////////////////////

// The following macros are used to specify the video update method when performing
// the video profile.  It is up to the user to voluntarily call the profile function
// to determine the fastest update method and set it accordingly.  Otherwise, 
// the default method will be determined by profiling fullscreen updates.
#define RSP_MAC_VIDEO_COPYBITS	0		// Use the Mac OS copybits for drawing to screen.
#define RSP_MAC_VIDEO_DIRECT		1		// Use RSPiX direct-to-screen routines.

extern int32_t rspMacVideoProfile(			// Returns average time per update.
	int16_t	sX,									// In:  X coord of rectangle region to profile
	int16_t	sY,									// In:  Y coord of rectangle region to profile
	int16_t	sWidth,								// In:  Width of the rectangle region
	int16_t	sHeight,								// In:  Height of the rectangle region
	int32_t	lTimeProfile,						// In:  Duration of time to do the update profile
	int16_t	sMethod);							// In:  Method to perform the profile.

extern void rspSetMacVideoUpdateOptions(
	int16_t	sMethod,								// In:  Method to be used
	int16_t sByteAlignment);					// In:  Byte alignment to be used

extern void rspQueryVideoModeReset(void);

extern int16_t rspQueryVideoMode(			// Returns 0 for each valid mode, then non-zero thereafter
	int16_t* psDeviceDepth,					// Out: Device depth (unless NULL)
	int16_t* psDeviceWidth = NULL,			// Out: Device width (unless NULL)
	int16_t* psDeviceHeight = NULL,			// Out: Device height (unless NULL)
	int16_t* psDevicePages = NULL);			// Out: Maximum number of pages supported (unless NULL)

extern int16_t rspGetVideoMode(				// Returns 0 if sucessfull, non-zero otherwise
	int16_t* psDeviceDepth,					// Out: Device depth (unless NULL)
	int16_t* psDeviceWidth = NULL,			// Out: Device width (unless NULL)
	int16_t* psDeviceHeight = NULL,			// Out: Device height (unless NULL)
	int16_t* psDevicePages = NULL,			// Out: Maximum number of pages supported (unless NULL)
	int16_t* psWidth = NULL,					// Out: Window width or -1 (unless NULL)
	int16_t* psHeight = NULL,					// Out: Window height or -1 (unless NULL)
	int16_t* psPages = NULL,					// Out: Number of pages or -1 (unless NULL)
	int16_t* psScaling = NULL);				// Out: Scaling flag or -1 (unless NULL)

extern int16_t rspSetVideoMode(				// Returns 0 if successfull, non-zero otherwise
	int16_t sDeviceDepth,						// In:  Device depth
	int16_t sDeviceWidth,						// In:  Device width
	int16_t sDeviceHeight,						// In:  Device height
	int16_t sWidth,								// In:  Window width
	int16_t sHeight,								// In:  Window height
	int16_t sPages = 1,							// In:  Number of pages to use
	int16_t sScaling = 0);						// In:  Scaling flag (0 = none, 1 = 2x scaling)

extern void rspKillVideoMode(void);

extern int16_t rspSuggestVideoMode(		// Returns 0 if successfull, non-zero otherwise
	int16_t		sDepth,							// In:  Required depth
	int16_t		sWidth,							// In:  Requested width
	int16_t		sHeight,							// In:  Requested height
	int16_t		sPages,							// In:  Required pages
	int16_t		sScaling,						// In:  Requested scaling
	int16_t*	psDeviceWidth = NULL,		// Out: Suggested device width (unless NULL)
	int16_t*	psDeviceHeight = NULL,		// Out: Suggested device height (unless NULL)
	int16_t*	psScaling = NULL);			// Out: Suggested scaling (unless NULL)

extern int16_t rspLockVideoPage(			// Returns 0 if successfull, non-zero otherwise
	void**	ppvMemory,						// Out: Pointer to video page or NULL
	int32_t*		plPitch);						// Out: Pitch of video page

extern void rspUnlockVideoPage(void);

extern int16_t rspLockVideoFlipPage(		// Returns 0 if successfull, non-zero otherwise
	void**	ppvMemory,						// Out: Pointer to video flip page or NULL
	int32_t*		plPitch);						// Out: Pitch of video flip page

extern void rspUnlockVideoFlipPage(void);

extern int16_t rspLockVideoBuffer(			// Returns 0 if successfull, non-zero otherwise
	void**	ppvBuffer,						// Out: Pointer to video buffer or NULL
	int32_t*		plPitch);						// Out: Pitch of video buffer

extern void rspUnlockVideoBuffer(void);

extern int16_t rspAllowPageFlip(void);	// Returns 0 if successfull, non-zero otherwise


extern void rspCacheDirtyRect(
	int16_t sX,					// x coord of upper-left corner of area to update
	int16_t sY,					// y coord of upper-left corner of area to update
	int16_t sWidth,				// Width of area to update
	int16_t sHeight);				// Height of area to update

extern void rspKeyRepeat(int bEnable);

extern void rspPresentFrame(void);
//Doesn't do anything
extern void rspUpdateDisplayRects(void);
//Doesn't do anything
extern void rspUpdateDisplay(void);
//Doesn't do anything
extern void rspUpdateDisplay(
	int16_t sX,									// In:  X coord of upper-left corner of area to update
	int16_t sY,									// In:  Y coord of upper-left corner of area to update
	int16_t sWidth,								// In:  Width of area to update
	int16_t sHeight);							// In:  Height of area to update

extern void rspSetPaletteEntry(
	int16_t sEntry,								// In:  Palette entry (0 to 255)
	uint8_t ucRed,						// In:  Red value (0 to 255)
	uint8_t ucGreen,					// In:  Green value (0 to 255)
	uint8_t ucBlue);					// In:  Blue value (0 to 255)

extern void rspSetPaletteEntries(
	int16_t sStartEntry,						// In:  Starting destination entry (0 to 255)
	int16_t sCount,								// In:  Number of entries to do (1 to 256)
	uint8_t* pucRed,					// In:  Pointer to starting source red value
	uint8_t* pucGreen,				// In:  Pointer to starting source green value
	uint8_t* pucBlue,					// In:  Pointer to starting source blue value
	int32_t lIncBytes);							// In:  What to add to pointers to move to next value

extern void rspGetPaletteEntries(
	int16_t sStartEntry,						// In:  Starting source entry (0 to 255)
	int16_t sCount,								// In:  Number of entries to do (1 to 256)
	uint8_t* pucRed,					// Out: Pointer to starting destination red value
	uint8_t* pucGreen,				// Out: Pointer to starting destination green value
	uint8_t* pucBlue,					// Out: Pointer to starting destination blue value
	int32_t lIncBytes);							// In:  What to add to pointers to move to next value

inline void rspGetPaletteEntry(
	int16_t sEntry,								// In:  Palette entry (0 to 255)
	uint8_t* pucRed,					// Out: Pointer to red value
	uint8_t* pucGreen,				// Out: Pointer to green value
	uint8_t* pucBlue)					// Out: Pointer to blue value
	{
	rspGetPaletteEntries(sEntry, 1, pucRed, pucGreen, pucBlue, 0);
	}

// Lock one or more palette entries.
//
// When an entry is locked, it prevents the entry from being changed by
// rspSetPaletteEntry() and rspSetPaletteEntries().
extern void rspLockPaletteEntries(
	int16_t sStartEntry,						// In:  Starting entry (0 to 255)
	int16_t sCount);								// In:  Number of entries to do (1 to 256)

// Unlock one or more palette entries.
//
// When an entry is unlocked, the entry can be changed by rspSetPaletteEntry()
// and rspSetPaletteEntries().
extern void rspUnlockPaletteEntries(
	int16_t sStartEntry,						// In:  Starting entry (0 to 255)
	int16_t sCount);								// In:  Number of entries to do (1 to 256)

// Set palette mapping tables.
//
// When rspUpdatePalette() is called, the current colors, set via
// rspSetPaletteEntrie(), are remapped through the specified maps, and the
// results are passed to the hardware palette.
//
// Note that this ONLY affects the hardware palette!  The colors that are set
// via rspSetPaletteEntries() are returned "intact" by rspGetPaletteEntires()!
extern void rspSetPaletteMaps(
	int16_t sStartEntry,						// In:  Starting destination entry (0 to 255)
	int16_t sCount,								// In:  Number of entries to do (1 to 256)
	uint8_t* pucRed,					// In:  Pointer to starting source red value
	uint8_t* pucGreen,				// In:  Pointer to starting source green value
	uint8_t* pucBlue,					// In:  Pointer to starting source blue value
	int32_t lIncBytes);							// In:  What to add to pointers to move to next value

// Set palette mapping tables (see rspSetPaletteMaps() for details.)
extern void rspGetPaletteMaps(
	int16_t sStartEntry,						// In:  Starting source entry (0 to 255)
	int16_t sCount,								// In:  Number of entries to do (1 to 256)
	uint8_t* pucRed,					// Out: Pointer to starting destination red value
	uint8_t* pucGreen,				// Out: Pointer to starting destination green value
	uint8_t* pucBlue,					// Out: Pointer to starting destination blue value
	int32_t lIncBytes);							// In:  What to add to pointers to move to next value
	
extern void rspUpdatePalette(void);

extern void rspShieldMouseCursor(void);

extern void rspUnshieldMouseCursor(void);

#define RSP_WHITE_INDEX		0
#define RSP_BLACK_INDEX		255


////////////////////////////////////////////////////////////////////////////////
// Time API
////////////////////////////////////////////////////////////////////////////////

extern int32_t rspGetMicroseconds(			// Returns time in microseconds
	int16_t sReset = FALSE);					// In:  TRUE to reset count, FALSE otherwise

extern int32_t rspGetMilliseconds(void);	// Returns time in milliseconds

extern S64 rspGetAppMicroseconds(void);	// Returns microseconds since app started


////////////////////////////////////////////////////////////////////////////////
// Sound API
////////////////////////////////////////////////////////////////////////////////

// Callback returns 0 if successfull, non-zero if no data was returned
typedef short (*RSP_SND_CALLBACK)(uint8_t*	pucBuffer,	// Data buffer to be filled
											int32_t		lSize,		// Size of buffer (must fill
																		// entire bufffer - pad with
																		// silence if necessary)
											int32_t		lDataPos,	// Data's starting position
																		// in sound out data stream
											uint32_t*	pulUser);	// For use by user (can be
																		// changed as desired)

extern int16_t rspSetSoundOutMode(				// Returns 0 if successfull, non-zero otherwise
	int32_t lSampleRate,								// In:  Sample rate
	int32_t lBitsPerSample,							// In:  Bits per sample
	int32_t lChannels,								// In:  Channels (mono = 1, stereo = 2)
	int32_t lCurBufferTime,							// In:  Current buffer time (in ms.)
	int32_t lMaxBufferTime,							// In:  Maximum buffer time (in ms.)
	RSP_SND_CALLBACK callback,					// In:  Callback function
	uint32_t ulUser);									// In:  User-defined value to pass to callback
	
extern int16_t rspGetSoundOutMode(				// Returns 0 if successfull, non-zero otherwise
	int32_t* plSampleRate,							// Out: Sample rate or -1 (unless NULL)
	int32_t* plBitsPerSample = NULL,				// Out: Bits per sample or -1 (unless NULL)
	int32_t* plChannels = NULL,					// Out: Channels (mono=1, stereo=2) or -1 (unless NULL)
	int32_t* plCurBufferTime = NULL,				// Out: Current buffer time or -1 (unless NULL)
	int32_t* plMaxBufferTime = NULL);			// Out: Maximum buffer time or -1 (unless NULL)

extern void rspSetSoundOutBufferTime(
	int32_t lCurBufferTime);						// In:  New buffer time

extern void rspKillSoundOutMode(void);		// Returns 0 if successfull, non-zero otherwise

extern int16_t rspClearSoundOut(void);		// Returns 0 on success, non-zero otherwise

extern int16_t rspPauseSoundOut(void);		// Returns 0 on success, non-zero otherwise

extern int16_t rspResumeSoundOut(void);		// Returns 0 on success, non-zero otherwise

extern int16_t rspIsSoundOutPaused(void);	// Returns TRUE if paused, FALSE otherwise

extern int32_t rspGetSoundOutPos(void);		// Returns sound output position in bytes

extern int32_t rspGetSoundOutTime(void);		// Returns sound output position in time

extern int32_t rspDoSound(void);

extern void rspLockSound(void);

extern void rspUnlockSound(void);

////////////////////////////////////////////////////////////////////////////////
// Application API (Really CYAN, but requires too much integration in BLUE)
////////////////////////////////////////////////////////////////////////////////

extern void rspSetApplicationName(
	char* pszName);								// In: Application name


////////////////////////////////////////////////////////////////////////////////
// Background API (Really CYAN, but requires too much integration in BLUE)
////////////////////////////////////////////////////////////////////////////////

extern int16_t rspIsBackground(void);			// Returns TRUE if in background, FALSE otherwise

extern void rspSetBackgroundCallback(
	void (BackgroundCallback)(void));		// In:  Function to be called

extern void rspSetForegroundCallback(
	void (ForegroundCallback)(void));		// In:  Function to be called


extern int rspCommandLine(const char *cmd);
extern void rspPlatformInit(void);

#endif // UNIXBLUE_H

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
