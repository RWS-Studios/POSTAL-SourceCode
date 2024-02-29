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
// Menu.H
// 
// History:
//		01/19/97 JMI	Started tracking history of this file.
//
//		01/19/97	JMI	Converted Do() to taking an RInputEvent* instead of a
//							long*.
//							Also, kept inline version of DoMenu() that takes a long*
//							and converts it to an RInputEvent which is passed to the
//							real DoMenu().  This is only temporary for backward 
//							compatability.
//
//		01/27/97	JMI	Added parameters to MenuBackground indicating what colors
//							to set in the palette, if any.  Also, added two new query
//							functions (GetCurrentMenuBackground(), and 
//							GetCurrentMenuBox() ).
//
//		01/30/97	JMI	Now callback can return non-zero aborting menu right
//							away once started.
//
//		02/02/97	JMI	Now the GUI can be set to use transparent BLiT'ing via
//							the Menu struct.  Now MenuItemCall with -1 for sMenuItem
//							indicates change of menu focus (but no choice yet made).
//
//		02/03/97	JMI	Split DoMenu() into DoMenuInput() and DoMenuOutput() so
//							the input and output for menus can be done at different
//							points in the user loop or whatever.
//
//		03/31/97	JMI	Added pragma to avoid zero-sized array warning under
//							Microsoft compiler.
//
//		04/21/97	JMI	Added pmenuBack to MenuAutoItems.
//
//		04/24/97	JMI	Now supports the centering flag for header text and 
//							two new flags for shadowing the header and the items.
//							Now colors are specified as RGBA and background font
//							color must be transparent (not offered as an option).
//
//		07/01/97	JMI	Added sBackItem to MenuAutoItems so when we use pmenuBack
//							we can select the item last selected.
//
//		07/04/97	JMI	Upped the ami[] member to 25 elements so the input 
//							functions could fit.
//
//		07/05/97	JMI	Increased ami[] to 26 elements (forgot that one is
//							a terminator).
//
//		07/06/97	JMI	Added MenuColumnizeGuis to make the GUIs associated with
//							menu items appear in their own column.
//							Also, fixed bugs in MAKE_U32_COLOR().
//
//		08/10/97	JMI	Increased ami[] to 31 elements b/c I needed more for the
//							input settings menu.
//
//		08/14/97	JMI	MenuItemCall (fnChoice) now returns true to accept menu 
//							choice, false otherwise.
//
//		08/25/97	JMI	Added two new palette info entries for the range of
//							mappable entries.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef MENU_H
#define MENU_H

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
///////////////////////////////////////////////////////////////////////////////
#include "Blue.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/guiItem.h"
	#include "WishPiX/ResourceManager/resmgr.h"
#else
	#include "guiItem.h"
	#include "resmgr.h"
#endif

// If under Microsoft compiler . . .
#ifdef _MSC_VER
	// Disable: 'nonstandard extension used : zero-sized array in struct/union'.
	#pragma warning( disable : 4200 )
#endif // _MSC_VER

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

#ifdef SYS_ENDIAN_LITTLE
	#define MAKE_U32_COLOR(u8Red, u8Blue, u8Green, u8Alpha)	\
		( (u8Alpha << 0) | (u8Red << 8) | (u8Green << 16) | (u8Blue << 24) )
#else
	#define MAKE_U32_COLOR(u8Red, u8Blue, u8Green, u8Alpha)	\
		( (u8Alpha << 24) | (u8Red << 16) | (u8Green << 8) | (u8Blue << 0) )
#endif

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// Forward delcaration for sub structs of Menu that need to reference it
// as a pointer.
struct Menu;

typedef enum		// Flags for mfFlags for Menu structure.
	{
	// Position flags.
	MenuPosCenter			= 0x0001,	// Menu should be centered.
	MenuPosFullscreen		= 0x0002,	// Menu should be fullscreen.
	
	// Background image flags.
	MenuBackTiled			= 0x0004,	// Background image should be tiled.

	// Text flags.
	MenuItemTextShadow	= 0x0008,	// Menu item strings should be shadowed.
	MenuHeaderTextCenter	= 0x0010,	// Menu header string should be centered.
	MenuHeaderTextShadow	= 0x0020,	// Menu header string should be shadowed.

	// GUI flags.
	MenuColumnizeGuis		= 0x0100		// GUIs should appear in their own column.
	} MenuFlags;

typedef struct
	{
	short		sX;	// X coordinate of menu.
	short		sY;	// Y coordinate of menu.
	short		sW;	// Width of menu.
	short		sH;	// Height of menu.

	short		sHeaderX;		// X position of menu header text.
									// Negative indicates offset from center.
	short		sHeaderY;		// Y position of menu header text.
									// Negative indicates offset from center.

	short		sItemX;			// X position of menu items w/i menu.
									// Negative indicates offset from center.
	short		sItemY;			// Y position of first menu item w/i menu.
									// Negative indicates offset from center.
	short		sItemSpacingY;	// Space between items vertically.

	short		sIndicatorSpacingX;	// Distance between indicator's right edge
											// and menu items' left edges.

	short		sMaxItemX;		// X position menu items should not pass w/i Menu.
									// Less than 1, indicates offset from right edge.
	short		sMaxItemY;		// Y position menu items should not pass w/i Menu.
									// Less than 1, indicates offset from right edge.
										
	} MenuPos;

typedef struct
	{
	char*			pszFile;				// Filespec of image for background of menu or NULL.
	U32			u32BackColor;		// Background color.
	short			sSetStartIndex;	// Starting index of palette entries to set.
	short			sSetNumEntries;	// Number of palette entries to set.
	short			sMapStartIndex;	// Starting index of palette entries that can be mapped to.
	short			sMapNumEntries;	// Number of palette entries that can be mapped to.
	} MenuBackground;

typedef struct
	{
	short	sTransparent;			// Set GUI to use transparent BLiT'ing.
	} MenuGui;

typedef struct
	{
	char*	pszFile;				// Filespec of font to use for menu item text.
	short	sHeight;				// Height to use for menu item text.
	U32	u32ForeColor;		// Color or color index for font to use for menu item
									// text.
	U32	u32ShadowColor;	// Color or color index for font to use for shadow
									// of menu item text.
	} MenuItemsFont;

typedef struct
	{
	char*	pszHeaderText;		// Header text.
	char*	pszFontFile;		// Filespec of font to use for menu header text.
	short	sHeight;				// Height to use for menu header text.
	U32	u32ForeColor;		// Color or color index for font to use for menu header
									// text.
	U32	u32ShadowColor;	// Color or color index for font to use for shadow
									// of menu header text.
	} MenuHeader;

typedef struct
	{
	char*	pszFile;			// Filespec of indicator image to use for current
								// selection.
	RImage::Type	type;	// New type to convert image to after load or 
								// RImage::NOT_SUPPORTED to skip conversion.
	} MenuIndicator;

typedef struct
	{
	short	sDefaultItem;	// Menu item (index in ami[]) selected initially.
								// Negative indicates distance from number of items
								// (e.g., -1 is the last item).
	short	sCancelItem;	// Menu item (index in ami[]) chosen on cancel.
								// Negative indicates distance from number of items
								// (e.g., -1 is the last item).
	Menu*	pmenuBack;		// If not NULL, the menu to go back to in the case
								// the cancel item (sCancelItem) is chosen.
	short	sBackItem;		// Last item selected on this menu.  Only use, if
								// getting to this menu via pmenuBack.
	} MenuAutoItems;

// Callback just before a menu is started.
typedef short (*MenuInitCall)(	// Returns 0 on success, non-zero to cancel menu.
	Menu*	pmenuCurrent,				// Current menu.
	short	sInit);						// TRUE, if initializing; FALSE, if killing.

// Callback for when a menu item is chosen or focused.
typedef bool (*MenuItemCall)(	// Returns true to accept choice, false otherwise.
	Menu*	pmenuCurrent,			// Current menu.
	short sMenuItem);				// Indicates menu item chosen, -1 indicates change
										// of focused menu item (but no choice yet taken).

typedef struct
	{
	MenuInitCall	fnInit;
	MenuItemCall	fnChoice;
	} MenuCallbacks;

typedef struct
	{
	char*			pszText;		// Text for menu item.
	short			sEnabled;	// TRUE if item is enabled, FALSE if disabled.
	Menu*			pmenu;		// Menu this item leads to or NULL.
	RGuiItem*	pgui;			// GuiItem to appear after text.
	} MenuItem;

struct Menu		// Structure defining a menu.
	{
	// User defined identifier.
	U32				u32Id;	// User may used to identify this menu.

	// Position info.
	MenuPos			menupos;

	// Background info.
	MenuBackground	menuback;

	// GUI settings.
	MenuGui			menugui;

	// Flags.
	MenuFlags		menuflags;

	// Header text and font.
	MenuHeader		menuheader;

	// Font for menu items.
	MenuItemsFont	menuitemsfont;

	// Indicator.
	MenuIndicator	menuindicator;

	// Callbacks.
	MenuCallbacks	menucallbacks;

	// Items chosen or selected automatically under certain conditions.
	MenuAutoItems	menuautoitems;

	// Menu item descriptors.  In order.  "" for empty lines.  NULL to end.
	// Note: Metrowerks Codewarrior compiler doesn't like unsized arrays, so
	//       this had to be given a size.  For now, 10 lines seems reasonable.
	//	But, alas, that was not enough so I upped it to 4096 and then I thought
	// that won't be enough so I did 32768 but that was too big and then I
	// noticed the poppa bear was coming home so I quickly backed it down to
	// 25 (which seems extremely reasonable next to 32768) and checked it in
	// without telling Mike.
	// And, then, later I increased it to 26 cuz I forgot one is the 
	// terminator.
	// And, then, I increased it to 31 b/c I needed more, More, MORE.
	MenuItem			ami[64];

	};


//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

// Sets up the menu described to be run via DoMenu() calls.
extern short StartMenu(				// Returns 0 on success.
	Menu*	pmenu,						// In: Pointer to Menu describing menu.
	RResMgr* presmgr,					// In:  Resource manager ptr.
	RImage*	pimDst);					// In:  Src for erase data.  Erase data is
											// updated back over the menu on StopMenu()
											// calls.

// Performs one iteration of menu input logic.  Call this repeatedly in
// conjunction with DoMenuOutput() to run the menu.
extern void DoMenuInput(	// Returns nothing.
	RInputEvent* pie,		// In:  Most recent user input event.
	short UseJoystick);

// Performs one iteration of menu output logic.  Call this repeatedly in
// conjunction with DoMenuInput() to run the menu.
extern void DoMenuOutput(	// Returns nothing.
	RImage*	pimDst);			// In:  Destination image for menu BLiTs.

// Get the current menu.
extern Menu* GetCurrentMenu(void);	// Returns a pointer to the current
												// menu or NULL if there is none.

// Call this to release all memory being used by the menu system
// It should be safe to call this function even if StartMenu()  
// failed, and it should be safe to call it multiple times -- in
// both cases, it shouldn't do anything.                        
extern short StopMenu(void);		// Returns 0 on success.

// Get the Menu's background image.
extern RImage* GetCurrentMenuBackground(void);	// Returns a pointer to the
																// current background image
																// or NULL, if none.

// Get the Menu's GUI.
extern RGuiItem* GetCurrentMenuBox(void);	// Returns a pointer to the current
														// menu GUI or NULL, if none.

#endif	// MENU_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
