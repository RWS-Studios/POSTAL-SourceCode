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

extern short rspInitBlue(void);

extern void rspKillBlue(void);

#define RSP_DOSYSTEM_HOGCPU		0
#define RSP_DOSYSTEM_TOLERATEOS	1
#define RSP_DOSYSTEM_SLEEP			2

extern void rspSetDoSystemMode(
	short sMode);									// In:  Mode (use RSP_DOSYSTEM_* macros)

extern void rspDoSystem(void);

// Global variable that lets app set the number of blocks that it guesses it
// might allocate from th heap.  This may help prevent fragmentation.  We set
// the default value, and it's up to the app to change it before calling
// rspInitBlue() if it wants to make a better guess.
extern short macGuessTotalHeapBlocks;

// Global variable that lets app set the amount of memory (in bytes) to reserve.
// Once the app has exhausted all other memory, a warning will be displayed
// saying that memory is very low.  The reserved memory is then freed, and the
// application is allowed to continue.  If the application exhausts all memory
// again, an error dialog is displayed and the application is terminated.
// NOTE: Currently used only with SmartHeap -- see mmain.cpp for details.
extern long macReserveMemBytes;


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
extern short rspAssert(	// Returns result.
	char* pszFile,			// Source file.
	short sLine,			// Source line.
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
	short* psX,				// X position returned here (unless NULL)
	short* psY,				// Y position returned here (unless NULL)
	short* psButtons);	// button status returned here (unless NULL)

extern void rspSetMouse(
	short sX,				// New x position
	short sY);				// New y position

extern short rspGetMouseEvent(	// Returns 0 if no event was available, non-zero otherwise
	short* psX,							// Event's X position is returned here (unless NULL)
	short* psY,							// Event's Y position is returned here (unless NULL)
	short* psButton,					// Event's button status is returned here (unless NULL)
	long* plTime = NULL,					// Event's time stamp returned here (unless NULL)
	short* psType = NULL);			// Event's type (as per OS) is returned here (unless NULL)

extern short rspGetLastMouseEvent(	// Returns 0 if no event was available, non-zero otherwise
	short* psX,								// Event's X position is returned here (unless NULL)
	short* psY,								// Event's Y position is returned here (unless NULL)
	short* psButton,						// Event's button status is returned here (unless NULL)
	long* plTime = NULL,					// Event's time stamp returned here (unless NULL)
	short* psType = NULL);				// Event's type (as per OS) is returned here (unless NULL)

extern void rspClearMouseEvents(void);

extern void rspHideMouseCursor(void);

extern void rspShowMouseCursor(void);

// Get the current cursor show level.  A level of 1 or greater means the cursor
// is currently showing, while a level of 0 or less means the cursor is hidden.
extern short rspGetMouseCursorShowLevel(void);

// Set the current cursor show level.  A level of 1 or greater means the cursor
// is currently showing, while a level of 0 or less means the cursor is hidden.
extern void rspSetMouseCursorShowLevel(
	short sLevel);												// In:  New cursor level

// Global variables for setting maximum mouse movement between two events,
// beyond which they would no longer be considered double-clicks.  These are
// set by this module to reasonable values, but an application CAN change
// them.  This is, however, a MAC-SPECIFIC EXTENSION, so user beware!
extern short mMouseDoubleClickX;
extern short mMouseDoubleClickY;


////////////////////////////////////////////////////////////////////////////////
// Keyboard API
////////////////////////////////////////////////////////////////////////////////

extern void rspScanKeys(
	unsigned char* pucKeys);				// Out: Array of 128 unsigned chars (one per SK code)

extern void rspClearKeyEvents(void);

extern short rspGetKey(						// Returns 1 if key was available, 0 if not
	long* plKey,								// Out: Key info (0 if no key was available)
	long* plTime = NULL);					// Out: Key's time stamp (unless NULL)

extern short rspIsKey(void);				// Returns 1 if key is available, 0 if not

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
extern long rspGetToggleKeyStates(void);	// Returns toggle key state flags

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
	long	lX;
	long	lY;
	long	lZ;
	long	lW;
	long	lU;
	long	lV;
	} RJoyPos;

///////////////////////////////////////////////////////////////////////////////
// JOYSTICK Prototypes.

// Updates joystick sJoy's current state and makes the current state the 
// previous.
extern void rspUpdateJoy(
		short sJoy);	// In:  Joystick to query.

// Puts the coordinates of joystick sJoy's position in your longs.
// This function returns directions in an analog format (0..0xFFFF).
extern void rspGetJoyPos(
	short sJoy,					// In:  Joystick to query.
	long *plX,					// Out: X axis position of joystick, if not NULL.
	long *plY = NULL,			// Out: Y axis position of joystick, if not NULL.
	long *plZ = NULL,			// Out: Z axis position of joystick, if not NULL.
	long *plW = NULL,			// Out: W axis position of joystick, if not NULL.
	long *plU = NULL,			// Out: U axis position of joystick, if not NULL.
	long *plV = NULL);		// Out: V axis position of joystick, if not NULL.

extern void rspGetJoyPos(
	short sJoy,					// In:  Joystick to query.
	RJoyPos* pjoypos);		// In:  Joystick positions for all axes.

// Puts the coordinates of the previous joystick sJoy's position in your longs.
// This function returns directions in an analog format (0..0xFFFF).
extern void rspGetJoyPrevPos(
	short sJoy,					// In:  Joystick to query.
	long *plX,					// Out: X axis position of joystick, if not NULL.
	long *plY = NULL,			// Out: Y axis position of joystick, if not NULL.
	long *plZ = NULL,			// Out: Z axis position of joystick, if not NULL.
	long *plW = NULL,			// Out: W axis position of joystick, if not NULL.
	long *plU = NULL,			// Out: U axis position of joystick, if not NULL.
	long *plV = NULL);		// Out: V axis position of joystick, if not NULL.

extern void rspGetJoyPrevPos(
	short sJoy,					// In:  Joystick to query.
	RJoyPos* pjoypos);		// In:  Joystick positions for all axes.

// Reads the joystick sJoy's current state.
// This function returns directions in a digital format (up, down, centered).
extern void rspGetJoyState(
	short sJoy,					// In:  Joystick to query.
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
	short sJoy,					// In:  Joystick to query.
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
extern short JoyBitfieldToIndex(U32 bitfield);
extern U32 JoyIndexToBitfield(short index);
extern short MouseBitfieldToIndex(U32 bitfield);
extern U32 MouseIndexToBitfield(short index);

////////////////////////////////////////////////////////////////////////////////
// DISPLAY API
////////////////////////////////////////////////////////////////////////////////

// The following macros are used to specify the video update method when performing
// the video profile.  It is up to the user to voluntarily call the profile function
// to determine the fastest update method and set it accordingly.  Otherwise, 
// the default method will be determined by profiling fullscreen updates.
#define RSP_MAC_VIDEO_COPYBITS	0		// Use the Mac OS copybits for drawing to screen.
#define RSP_MAC_VIDEO_DIRECT		1		// Use RSPiX direct-to-screen routines.

extern long rspMacVideoProfile(			// Returns average time per update.
	short	sX,									// In:  X coord of rectangle region to profile
	short	sY,									// In:  Y coord of rectangle region to profile
	short	sWidth,								// In:  Width of the rectangle region
	short	sHeight,								// In:  Height of the rectangle region
	long	lTimeProfile,						// In:  Duration of time to do the update profile
	short	sMethod);							// In:  Method to perform the profile.

extern void rspSetMacVideoUpdateOptions(
	short	sMethod,								// In:  Method to be used
	short sByteAlignment);					// In:  Byte alignment to be used

extern void rspQueryVideoModeReset(void);

extern short rspQueryVideoMode(			// Returns 0 for each valid mode, then non-zero thereafter
	short* psDeviceDepth,					// Out: Device depth (unless NULL)
	short* psDeviceWidth = NULL,			// Out: Device width (unless NULL)
	short* psDeviceHeight = NULL,			// Out: Device height (unless NULL)
	short* psDevicePages = NULL);			// Out: Maximum number of pages supported (unless NULL)

extern short rspGetVideoMode(				// Returns 0 if sucessfull, non-zero otherwise
	short* psDeviceDepth,					// Out: Device depth (unless NULL)
	short* psDeviceWidth = NULL,			// Out: Device width (unless NULL)
	short* psDeviceHeight = NULL,			// Out: Device height (unless NULL)
	short* psDevicePages = NULL,			// Out: Maximum number of pages supported (unless NULL)
	short* psWidth = NULL,					// Out: Window width or -1 (unless NULL)
	short* psHeight = NULL,					// Out: Window height or -1 (unless NULL)
	short* psPages = NULL,					// Out: Number of pages or -1 (unless NULL)
	short* psScaling = NULL);				// Out: Scaling flag or -1 (unless NULL)

extern short rspSetVideoMode(				// Returns 0 if successfull, non-zero otherwise
	short sDeviceDepth,						// In:  Device depth
	short sDeviceWidth,						// In:  Device width
	short sDeviceHeight,						// In:  Device height
	short sWidth,								// In:  Window width
	short sHeight,								// In:  Window height
	short sPages = 1,							// In:  Number of pages to use
	short sScaling = 0);						// In:  Scaling flag (0 = none, 1 = 2x scaling)

extern void rspKillVideoMode(void);

extern short rspSuggestVideoMode(		// Returns 0 if successfull, non-zero otherwise
	short		sDepth,							// In:  Required depth
	short		sWidth,							// In:  Requested width
	short		sHeight,							// In:  Requested height
	short		sPages,							// In:  Required pages
	short		sScaling,						// In:  Requested scaling
	short*	psDeviceWidth = NULL,		// Out: Suggested device width (unless NULL)
	short*	psDeviceHeight = NULL,		// Out: Suggested device height (unless NULL)
	short*	psScaling = NULL);			// Out: Suggested scaling (unless NULL)

extern short rspLockVideoPage(			// Returns 0 if successfull, non-zero otherwise
	void**	ppvMemory,						// Out: Pointer to video page or NULL
	long*		plPitch);						// Out: Pitch of video page

extern void rspUnlockVideoPage(void);

extern short rspLockVideoFlipPage(		// Returns 0 if successfull, non-zero otherwise
	void**	ppvMemory,						// Out: Pointer to video flip page or NULL
	long*		plPitch);						// Out: Pitch of video flip page

extern void rspUnlockVideoFlipPage(void);

extern short rspLockVideoBuffer(			// Returns 0 if successfull, non-zero otherwise
	void**	ppvBuffer,						// Out: Pointer to video buffer or NULL
	long*		plPitch);						// Out: Pitch of video buffer

extern void rspUnlockVideoBuffer(void);

extern short rspAllowPageFlip(void);	// Returns 0 if successfull, non-zero otherwise


extern void rspCacheDirtyRect(
	short sX,					// x coord of upper-left corner of area to update
	short sY,					// y coord of upper-left corner of area to update
	short sWidth,				// Width of area to update
	short sHeight);				// Height of area to update

extern void rspKeyRepeat(int bEnable);

extern void rspPresentFrame(void);

extern void rspUpdateDisplayRects(void);

extern void rspUpdateDisplay(void);

extern void rspUpdateDisplay(
	short sX,									// In:  X coord of upper-left corner of area to update
	short sY,									// In:  Y coord of upper-left corner of area to update
	short sWidth,								// In:  Width of area to update
	short sHeight);							// In:  Height of area to update

extern void rspSetPaletteEntry(
	short sEntry,								// In:  Palette entry (0 to 255)
	unsigned char ucRed,						// In:  Red value (0 to 255)
	unsigned char ucGreen,					// In:  Green value (0 to 255)
	unsigned char ucBlue);					// In:  Blue value (0 to 255)

extern void rspSetPaletteEntries(
	short sStartEntry,						// In:  Starting destination entry (0 to 255)
	short sCount,								// In:  Number of entries to do (1 to 256)
	unsigned char* pucRed,					// In:  Pointer to starting source red value
	unsigned char* pucGreen,				// In:  Pointer to starting source green value
	unsigned char* pucBlue,					// In:  Pointer to starting source blue value
	long lIncBytes);							// In:  What to add to pointers to move to next value

extern void rspGetPaletteEntries(
	short sStartEntry,						// In:  Starting source entry (0 to 255)
	short sCount,								// In:  Number of entries to do (1 to 256)
	unsigned char* pucRed,					// Out: Pointer to starting destination red value
	unsigned char* pucGreen,				// Out: Pointer to starting destination green value
	unsigned char* pucBlue,					// Out: Pointer to starting destination blue value
	long lIncBytes);							// In:  What to add to pointers to move to next value

inline void rspGetPaletteEntry(
	short sEntry,								// In:  Palette entry (0 to 255)
	unsigned char* pucRed,					// Out: Pointer to red value
	unsigned char* pucGreen,				// Out: Pointer to green value
	unsigned char* pucBlue)					// Out: Pointer to blue value
	{
	rspGetPaletteEntries(sEntry, 1, pucRed, pucGreen, pucBlue, 0);
	}

// Lock one or more palette entries.
//
// When an entry is locked, it prevents the entry from being changed by
// rspSetPaletteEntry() and rspSetPaletteEntries().
extern void rspLockPaletteEntries(
	short sStartEntry,						// In:  Starting entry (0 to 255)
	short sCount);								// In:  Number of entries to do (1 to 256)

// Unlock one or more palette entries.
//
// When an entry is unlocked, the entry can be changed by rspSetPaletteEntry()
// and rspSetPaletteEntries().
extern void rspUnlockPaletteEntries(
	short sStartEntry,						// In:  Starting entry (0 to 255)
	short sCount);								// In:  Number of entries to do (1 to 256)

// Set palette mapping tables.
//
// When rspUpdatePalette() is called, the current colors, set via
// rspSetPaletteEntrie(), are remapped through the specified maps, and the
// results are passed to the hardware palette.
//
// Note that this ONLY affects the hardware palette!  The colors that are set
// via rspSetPaletteEntries() are returned "intact" by rspGetPaletteEntires()!
extern void rspSetPaletteMaps(
	short sStartEntry,						// In:  Starting destination entry (0 to 255)
	short sCount,								// In:  Number of entries to do (1 to 256)
	unsigned char* pucRed,					// In:  Pointer to starting source red value
	unsigned char* pucGreen,				// In:  Pointer to starting source green value
	unsigned char* pucBlue,					// In:  Pointer to starting source blue value
	long lIncBytes);							// In:  What to add to pointers to move to next value

// Set palette mapping tables (see rspSetPaletteMaps() for details.)
extern void rspGetPaletteMaps(
	short sStartEntry,						// In:  Starting source entry (0 to 255)
	short sCount,								// In:  Number of entries to do (1 to 256)
	unsigned char* pucRed,					// Out: Pointer to starting destination red value
	unsigned char* pucGreen,				// Out: Pointer to starting destination green value
	unsigned char* pucBlue,					// Out: Pointer to starting destination blue value
	long lIncBytes);							// In:  What to add to pointers to move to next value
	
extern void rspUpdatePalette(void);

extern void rspShieldMouseCursor(void);

extern void rspUnshieldMouseCursor(void);

#define RSP_WHITE_INDEX		0
#define RSP_BLACK_INDEX		255


////////////////////////////////////////////////////////////////////////////////
// Time API
////////////////////////////////////////////////////////////////////////////////

extern long rspGetMicroseconds(			// Returns time in microseconds
	short sReset = FALSE);					// In:  TRUE to reset count, FALSE otherwise

extern long rspGetMilliseconds(void);	// Returns time in milliseconds

extern S64 rspGetAppMicroseconds(void);	// Returns microseconds since app started


////////////////////////////////////////////////////////////////////////////////
// Sound API
////////////////////////////////////////////////////////////////////////////////

// Callback returns 0 if successfull, non-zero if no data was returned
typedef short (*RSP_SND_CALLBACK)(UCHAR*	pucBuffer,	// Data buffer to be filled
											long		lSize,		// Size of buffer (must fill
																		// entire bufffer - pad with
																		// silence if necessary)
											long		lDataPos,	// Data's starting position
																		// in sound out data stream
											ULONG*	pulUser);	// For use by user (can be
																		// changed as desired)

extern short rspSetSoundOutMode(				// Returns 0 if successfull, non-zero otherwise
	long lSampleRate,								// In:  Sample rate
	long lBitsPerSample,							// In:  Bits per sample
	long lChannels,								// In:  Channels (mono = 1, stereo = 2)
	long lCurBufferTime,							// In:  Current buffer time (in ms.)
	long lMaxBufferTime,							// In:  Maximum buffer time (in ms.)
	RSP_SND_CALLBACK callback,					// In:  Callback function
	ULONG ulUser);									// In:  User-defined value to pass to callback
	
extern short rspGetSoundOutMode(				// Returns 0 if successfull, non-zero otherwise
	long* plSampleRate,							// Out: Sample rate or -1 (unless NULL)
	long* plBitsPerSample = NULL,				// Out: Bits per sample or -1 (unless NULL)
	long* plChannels = NULL,					// Out: Channels (mono=1, stereo=2) or -1 (unless NULL)
	long* plCurBufferTime = NULL,				// Out: Current buffer time or -1 (unless NULL)
	long* plMaxBufferTime = NULL);			// Out: Maximum buffer time or -1 (unless NULL)

extern void rspSetSoundOutBufferTime(
	long lCurBufferTime);						// In:  New buffer time

extern void rspKillSoundOutMode(void);		// Returns 0 if successfull, non-zero otherwise

extern short rspClearSoundOut(void);		// Returns 0 on success, non-zero otherwise

extern short rspPauseSoundOut(void);		// Returns 0 on success, non-zero otherwise

extern short rspResumeSoundOut(void);		// Returns 0 on success, non-zero otherwise

extern short rspIsSoundOutPaused(void);	// Returns TRUE if paused, FALSE otherwise

extern long rspGetSoundOutPos(void);		// Returns sound output position in bytes

extern long rspGetSoundOutTime(void);		// Returns sound output position in time

extern long rspDoSound(void);

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

extern short rspIsBackground(void);			// Returns TRUE if in background, FALSE otherwise

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
