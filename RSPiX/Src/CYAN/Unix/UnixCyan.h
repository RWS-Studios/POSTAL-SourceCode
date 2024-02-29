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
///////////////////////////////////////////////////////////////////////////////
//
//	UnixCyan.h
// 
// History:
//		06/01/04 RCG     Initial add.
///////////////////////////////////////////////////////////////////////////////
#ifndef CYAN
#define CYAN

#ifndef WIN32
#include <sys/param.h>
#endif

#include "System.h"

///////////////////////////////////////////////////////////////////////////////
// Special Color API
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Set the RSPiX palette to the standard Win32 "static colors".
// If requested, the static colors will be locked.
//
// Note that the hardware palette is NOT affected by this function!
//
// Under Win32, the first 10 and last 10 palette colors are called the "static
// colors" because, historically, they were never changed.  Nowadays, they
// are often changed to create different desktop appearances or themes.  In
// any case, we often need some basic set of colors when creating cross-
// platform apps, and since Win32 is far more reliant on colors than the Mac
// (which can get by with just 2 colors) we chose the Win32 static colors as
// a common ground.  The RGB values for these colors were chosen based on the
// Win95/NT desktop appearance scheme called "Windows Standard".  These colors
// are very similar to those used by Windows 3.1 (which we support via Win32s)
// and those used by newer Mac apps that use the "3D" GUI look.
//
///////////////////////////////////////////////////////////////////////////////
void rspSetWin32StaticColors(
	short sLock = 0);										// In:  1 means lock colors, 0 means don't


///////////////////////////////////////////////////////////////////////////////
// Dialog API
///////////////////////////////////////////////////////////////////////////////

// MsgBox button flags and return types.
#define RSP_MB_BUT_OK					0x0000
#define RSP_MB_BUT_OKCANCEL			0x0001
#define RSP_MB_BUT_ABORTRETRYIGNORE	0x0002
#define RSP_MB_BUT_YESNOCANCEL		0x0003
#define RSP_MB_BUT_YESNO				0x0004
#define RSP_MB_BUT_RETRYCANCEL		0x0005
#define RSP_MB_BUT_MASK					0x000F

// MsgBox icon flags.
#define RSP_MB_ICN_STOP					0x0010
#define RSP_MB_ICN_QUERY				0x0020
#define RSP_MB_ICN_EXCLAIM				0x0030
#define RSP_MB_ICN_INFO					0x0040
#define RSP_MB_ICN_MASK					0x00F0

// MsgBox return values.
#define RSP_MB_RET_OK					0x0000
#define RSP_MB_RET_CANCEL				0x0001
#define RSP_MB_RET_ABORT				0x0002
#define RSP_MB_RET_RETRY				0x0003
#define RSP_MB_RET_IGNORE				0x0004
#define RSP_MB_RET_YES					0x0005
#define RSP_MB_RET_NO					0x0006

// Display a message (with buttons and/or icon if specified).
extern short rspMsgBox(	// Returns one of the macro defined RSP_MB_RET_*.
	USHORT usFlags,		// |'ed RSP_MB_BUT/ICO_* representing buttons and icons
								// desired.
	char *pszTitle,		// Title for box.
	char *pszFrmt,			// Format of string (sprintf flags).
	...);						// Various crap to represent sprintf flags.

////////////////////////////////////////////////////////////////////////////////
//
// Display an "Open File" dialog box.
//
// All paths and filenames are in system-dependant format, NOT RSPiX format!
//
// NOTE: It is acceptable for pszDefaultPath and pszSelectedFile to point to
// the same buffer!
//
// The pszDefaultPath parameter determines the directory that the dialog box
// starts out in.  It can specify a filename, too, but whether that shows up
// as the default in the dialog is system-dependant.
//
// If this function is successfull, the full path and filename that the user
// selected is copied to the buffer pointed to by pszSelectedFile.  If that
// buffer is not large enough to hold the full path and name, nothing is
// copied to the buffer and the function returns an error.
//
// The pszFilter paramter is optional.  If none is specified, or if an empty
// string is specified, then by default all files will be shown in the dialog.
// Note, however, that some systems allow the user to override this filtering
// and therefore to select ANY file.  In other words, do not rely on the filter
// to prevent the user from selecting some other type of file.
//
// In order to maintain some level of system independance, the filter mechanism
// is limited.  Wildcards such as "*" and "?" are NOT supported, and in fact,
// the very concept of filter "by extension" doesn't make sense on some systems.
// However, we need to go with something, so we'll assume that any of our
// "cross-platform" apps are going to use some kinds of extensions.
//
// For the filter, you can specify one or more extensions, each starting with
// a "." (period).  For instance, ".h" or ".cpp".  For multiple extensions,
// simply run them together, one after another: ".h.cpp.txt.bigextension".
// The maximum number of extensions allowed may vary by system, so keep it
// reasonable.  Keep the total length of the filter string reasonable, too.
// 
// Note that this dialog box only allows the user to select a FILE, not a
// directory, volume or drive.
//
////////////////////////////////////////////////////////////////////////////////
extern short rspOpenBox(								// Returns 0 if successfull, non-zero otherwise
	const char* pszBoxTitle,							// In:  Title of box
	const char*	pszDefaultPath,						// In:  Default directory and file
	char* pszSelectedFile,								// Out: File that user selected
	short sSelectedFileBufSize,						// In:  Size of buffer pointed to by pszSelectedFile
	const char*	pszFilter = NULL);					// In:  Filename filter or NULL for none

////////////////////////////////////////////////////////////////////////////////
//
// Display an "Save File" dialog box.
//
// Note: See rspOpenBox() documentation -- it's the same for this function!
//
////////////////////////////////////////////////////////////////////////////////
extern short rspSaveBox(								// Returns 0 if successfull, non-zero otherwise
	const char* pszBoxTitle,							// In:  Title of box
	const char*	pszDefaultPath,						// In:  Default directory and file
	char* pszSelectedFile,								// Out: File that user selected
	short sSelectedFileBufSize,						// In:  Size of buffer pointed to by pszSelectedFile
	const char*	pszFilter = NULL);					// In:  Filename filter or NULL for none


///////////////////////////////////////////////////////////////////////////////
// Exec API
///////////////////////////////////////////////////////////////////////////////

// Executes pszExe with pszDir as its current directory.  Note that the
// pszExe can be relative, but only to the current path.  The pszDir parameter
// is only for the current directory for the rspExec'd process.
extern short rspExec(			// Returns 0 on success.
	char*	pszExe,					// Executable (full path or relative to CURRENT).
	char* pszDir = NULL,			// Current drive/directory for the child process,
										// if not NULL.
	short	sWait = FALSE,			// Waits for child execution to stop before
										// returning if TRUE.
	short (*waitcall)(void)		// App callback to call during wait, if not NULL.
					= NULL);			// Returns 0 to continue waiting, 
										// non-zero otherwise.

///////////////////////////////////////////////////////////////////////////////
// File Path API
///////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#ifndef PATH_MAX  // !!! FIXME: where is this really defined?
#define PATH_MAX 260
#endif
#endif

#define RSP_MAX_PATH                        PATH_MAX
#define RSP_PATH_SEPARATOR				'/'

#ifdef WIN32
#define RSP_SYSTEM_PATH_SEPARATOR	'\\'
#else
#define RSP_SYSTEM_PATH_SEPARATOR	'/'
#endif

extern short rspGetTempPath(			// Returns 0 on success, non-zero otherwise
	char* pszPath,							// Out: Temp path returned here if available.
	short	sMaxPathLen);					// In:  Max path length (to avoid overwrites)

extern char* rspPathToSystem(			// Returns pszSystem
	const char* pszRSPiX,				// In:  RSPiX path
	char* pszSystem);						// Out: System path

extern char* rspPathToSystem(			// Returns pointer to system-specific path (static!!!)
	const char* pszRSPiX);				// In:  RSPiX path

extern char* rspPathFromSystem(		// Returns pszSystem
	const char* pszRSPiX,				// In:  RSPiX path
	char* pszSystem);						// Out: System path

extern char* rspPathFromSystem(		// Returns pointer to system-specific path (static!!!)
	const char* pszRSPiX);				// In:  RSPiX path


///////////////////////////////////////////////////////////////////////////////
// File Attributes API
///////////////////////////////////////////////////////////////////////////////

#define RSP_READPERMISSION		0x0001
#define RSP_WRITEPERMISSION	0x0002

extern short rspSetFileAttributes(	// Returns 0 on success, non-zero otherwise
	char* pszFileName,					// In:  Filename of file to update. 
	short sAttributes);					// Out: New attributes to be set.


///////////////////////////////////////////////////////////////////////////////
// Cursor API
///////////////////////////////////////////////////////////////////////////////

// Cursor ID's for built-in cursors (never need loading or unloading)
#define RSP_CURSOR_ARROW		((short)0)
#define RSP_CURSOR_IBEAM		((short)1)
#define RSP_CURSOR_CROSSHAIR	((short)2)
//#define RSP_CURSOR_BIGPLUS	((short)3)	// Not used for now
#define RSP_CURSOR_WAIT			((short)4)

// CURSOR type used to manipulate cursors
typedef struct
	{
	short sType;			// Indicates type of cursor (b&w or color) and also
								// whether this struct is valid.
	long lParam;			// Used as CursHandle or CCrsrHandle but declared as long
								// to avoid requiring the use of MacOS header files.
	} RCursor;

extern void rspSetCursor(
	short sCursorID);						// In:  ID of built-in cursor (use RSP_CURSOR_* macros)
	
extern void rspSetCursor(
	RCursor* pCursor);						// In:  Pointer to RCursor

extern short rspLoadCursor(			// Returns 0 if successfull, non-zero otherwise
	short sResourceID,					// In:  Resource ID of cursor (type 'CURS' or 'crsr')
	RCursor** ppCursor);					// Out: Pointer to RCursor returned here

extern void rspUnloadCursor(
	RCursor* pCursor);						// In:  RCursor to unload


///////////////////////////////////////////////////////////////////////////////
// Quit API
///////////////////////////////////////////////////////////////////////////////

extern short rspGetQuitStatus(void);	// Returns TRUE if quit detected, FALSE otherwise

extern void rspSetQuitStatus(
	short sStatus);							// In:  New status (TRUE or FALSE)

// Set which modifier keys must be pressed in addition to the whatever ones
// are normally associated with the system's "quit key".
extern void rspSetQuitStatusFlags(
	long lModifiers);							// In:  Modifier key flags (RSP_GKF_*)


///////////////////////////////////////////////////////////////////////////////
// Printer API
///////////////////////////////////////////////////////////////////////////////

extern short rspSetPrinterResolution(		// Returns 0 if successfull, non-zero otherwise
	long lReqHorzRes,								// In:  Requested horizontal resolution (dpi)
	long lReqVertRes,								// In:  Requested vertical resolution (dpi)
	short sReqSquareRes);						// In:  Requested square resolution (TRUE or FALSE)

extern short rspPrinterPageSetupDialog(void);	// Returns 0 if successfull, non-zero otherwise

extern short rspPrinterPrintDialog(void);	// Returns 0 if successfull, non-zero otherwise

extern short rspGetPrinterResolution(		// Returns 0 if successfull, non-zero otherwise
	long* plHorzRes,								// Out: Horizontal resolution (dpi)
	long* plVertRes);								// Out: Vertical resolution (dpi)

extern short rspGetPrinterPageSize(			// Returns 0 if successfull, non-zero otherwise
	long* plPageWidth,							// Out: Page width (in dots)
	long* plPageHeight);							// Out: Page height (in dots)

extern short rspGetPrinterPageRotation(	// Returns 0 if successfull, non-zero otherwise
	short* psRotation);							// Out: Rotation (0, 90, or 270 degrees)

extern short rspGetPrinterPageMargins(		// Returns 0 if successfull, non-zero otherwise
	long* plLeftMargin,							// Out: Page's left margin (in dots)
	long* plRightMargin,							// Out: Page's right margin (in dots)
	long* plTopMargin,							// Out: Page's top margin (in dots)
	long* plBottomMargin);						// Out: Page's bottom margin (in dots)

extern short rspGetPrinterJob(				// Returns 0 if successfull, non-zero otherwise
	short* psFirstPage,							// Out: First page of range to be printed
	short* psLastPage,							// Out: Last page of range to be printed
	short* psCopies);								// Out: Copies to be printed

extern short rspStartPrinterDoc(void);		// Returns 0 if successfull, non-zero otherwise

extern void rspEndPrinterDoc(void);

extern short rspStartPrinterPage(void);	// Returns 0 if successfull, non-zero otherwise

extern short rspEndPrinterPage(void);

extern short rspPrintToPage(					// Returns 0 if successfull, non-zero otherwise
	U8* pu8Src,										// In:  Source data
	long lPitch,									// In:  Source pitch
	long lWidth,									// In:  Source width
	long lHeight,									// In:  Source height
	short sDepth,									// In:  Source depth (1 or 8)
	long lSrcX,										// In:  Source (image) X coord
	long lSrcY,										// In:  Source (image) Y coord
	long lSrcW,										// In:  Source (image) width
	long lSrcH,										// In:  Source (image) height
	long lDstX,										// In:  Destination (page) X coord
	long lDstY,										// In:  Destination (page) Y coord
	long lDstW,										// In:  Destination (page) width
	long lDstH);									// In:  Distination (page) height

extern short rspPrintToPage(					// Returns 0 if successfull, non-zero otherwise
	U8* pu8Src,										// In:  Source data
	long lPitch,									// In:  Source pitch
	long lWidth,									// In:  Source width
	long lHeight,									// In:  Source height
	short sDepth,									// In:  Source depth (1 or 8 - if 8, palette info must be valid!)
	short sSrcPalStartIndex,					// In:  Starting palette index (0 to 255)
	short sSrcPalEntries,						// In:  Number of palette entries (1 to 256)
	U8* pu8SrcPalRed,								// In:  Pointer to starting source red value
	U8* pu8SrcPalGreen,							// In:  Pointer to starting source green value
	U8* pu8SrcPalBlue,							// In:  Pointer to starting source blue value
	long lSrcPalIncBytes,						// In:  What to add to pointers to move to next value
	long lSrcX,										// In:  Source (image) X coord
	long lSrcY,										// In:  Source (image) Y coord
	long lSrcW,										// In:  Source (image) width
	long lSrcH,										// In:  Source (image) height
	long lDstX,										// In:  Destination (page) X coord
	long lDstY,										// In:  Destination (page) Y coord
	long lDstW,										// In:  Destination (page) width
	long lDstH);									// In:  Distination (page) height

#endif // CYAN
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
