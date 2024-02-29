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
// menu.CPP
// 
// History:
//		11/27/96 JMI	Started.
//
//		12/16/96	JMI	While not complete, this now contains most of the
//							features discussed.
//
//		12/21/96	JMI	Now uses RResMgr for all loaded resources except
//							ms_guiIndicator.m_im since it is not yet practical.
//							Once there is a generic loadable type, such as RFile*
//							or void*, this will be easier.  The possible work
//							around for the meantime could be a func to load an
//							image resource, allocate the gui via Create() to
//							the size of that resource, rspBlit() the image resource
//							into the gui, and Release() the image resource.
//							
//		12/21/96	JMI	Now calls ms_msgbox.SetVisible(TRUE) to show and activate
//							ms_msgbox and its children at the end of a successful
//							StartMenu().  Also, calls ms_msgbox.SetVisible(FALSE) to
//							hide and deactivate ms_msgbox and its children during
//							StopMenu().
//
//		01/19/97	JMI	Converted Do() to taking an RInputEvent* instead of a
//							long*.
//
//		01/27/97	JMI	NextItem() was bailing out one too early when it thought
//							it detected that there were no enabled menu items.
//
//		01/27/97	JMI	Now sets palette to background image.
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
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/04/97	JMI	Callback on StopMenu() now occurs after all deallocations.
//
//		02/10/97	JMI	rspReleaseResource() now takes a ptr to a ptr.
//
//		04/08/97	JMI	Now sucks up (and utilizes) Tab and Shift-Tab combos.
//
//		04/11/97	JMI	Now sets the new m_sFontCellHeight field in each GUI
//							that displays text in a menu.
//
//		04/21/97	JMI	Added pmenuBack to MenuAutoItems.
//
//		04/24/97	JMI	Now supports the centering flag for header text and 
//							two new flags for shadowing the header and the items.
//							Now colors are specified as RGBA and background font
//							color must be transparent (not offered as an option).
//
//		06/16/97	JMI	Now stores the current composite buffer contents on
//							StartMenu() and restores them on StopMenu().
//
//		07/01/97	JMI	Added sBackItem to MenuAutoItems so when we use pmenuBack
//							we can select the item last selected.
//
//		07/06/97	JMI	Added MenuColumnizeGuis to make the GUIs associated with
//							menu items appear in their own column.
//
//		07/21/97	JMI	Now displays disabled items with shadow and text colors
//							swapped.
//
//		08/10/97	JMI	Made MAX_MENU_ITEMS based on sizeof ami array.
//
//		08/14/97	JMI	MenuItemCall (fnChoice) now returns true to accept menu 
//							choice, false otherwise.
//
//		08/22/97	JMI	Now locks the composite buffer before accessing it.
//
//		08/25/97	JMI	Added two new palette info entries for the range of
//							mappable entries.
//
//		08/27/97	JMI	Now the cancel item cannot be disabled...good thing?
//							Probably...otherwise menus cancelled by callbacks and 
//							such would have no where to go which was kind of worrying
//							me before.  Also, Useful for having no actual cancel item.
//
//////////////////////////////////////////////////////////////////////////////
//
// This module operates a menu specified by the user via a structure, Menu.
// There are only a few basic calls, StartMenu(), DoMenuInput(), 
// DoMenuOutput(), GetCurrentMenu(), and StopMenu().  Communication back to 
// the caller is via callbacks.  
//
// The user must define a Menu struct and pass a pointer to it to StartMenu().
// At this point, the StopMenu() will automatically be called to destroy any
// existing menu.  Next, the new menu described by the Menu struct is created.
// Now, iteratively, the DoMenuInput() and DoMenuOutput() functions should be
// called to service the menu.  This will process the supplied input, if it
// pertains to the menu, and draw the menu into the supplied image.  Finally,
// when done with the menu, call StopMenu() to clean up the current menu.  
// Calls to DoMenu*() when no menu is active will simply return.
//
// Note regarding RResMgr.  The only members of the RResMgr utilized are
// Load(...) and Release(...).  This means that the user of the Menu API is
// resposible for the other stuff, such as OpenSak(), CloseSak(), Purge(),
// etc.
//
//////////////////////////////////////////////////////////////////////////////

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
#include "input.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/color/colormatch.h"
	#include "ORANGE/MsgBox/MsgBox.h"
	#include "CYAN/cyan.h"
#else
	#include "colormatch.h"
	#include "MsgBox.h"
	#include "cyan.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Application headers.
//////////////////////////////////////////////////////////////////////////////
#include "menu.h"

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)	(sizeof(a) / sizeof(a[0]) )

#define MAX_MENU_ITEMS	short(NUM_ELEMENTS(ms_pmenu->ami) - 1)

// Determines the indicator's Y position based on the supplied menu item index.
#define INDICATOR_POS_Y(sIndex)	(ms_asMenuItemPosY[sIndex] - \
											ms_guiIndicator.m_im.m_sHeight / 2)

// Determines the indiator's X position based on the supplied menu item index.
#define INDICATOR_POS_X(sIndex)	(ms_asMenuItemPosX[sIndex] - \
											ms_guiIndicator.m_im.m_sWidth - \
											ms_pmenu->menupos.sIndicatorSpacingX) 

// Determines an item index from a positive index or a negative offset from
// the number of items.
#define ITEMINDEX(sIndex)	(((sIndex) < 0) ? ms_sNumMenuItems + (sIndex) : (sIndex))

#define INVALID_MENU_ITEM	0x7fff

#define SHADOW_X_PIXELS	1
#define SHADOW_Y_PIXELS	1

#ifdef SYS_ENDIAN_LITTLE
	#define ALPHA(u32)	((u32 & 0x000000FF) >> 0)
	#define RED(u32)		((u32 & 0x0000FF00) >> 8)
	#define GREEN(u32)	((u32 & 0x00FF0000) >> 16)
	#define BLUE(u32)		((u32 & 0xFF000000) >> 24)
#else
	#define ALPHA(u32)	((u32 & 0xFF000000) >> 24)
	#define RED(u32)		((u32 & 0x00FF0000) >> 16)
	#define GREEN(u32)	((u32 & 0x0000FF00) >> 8)
	#define BLUE(u32)		((u32 & 0x000000FF) >> 0)
#endif

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

static Menu*		ms_pmenu	= NULL;			// Pointer to current menu.

static RMsgBox		ms_msgbox;					// Menu container.

static short		ms_sNumMenuItems	= 0;	// Current number of menu items.

static RFont*		ms_pfontItems	= NULL;	// Font for menu items.
static RFont*		ms_pfontHeader	= NULL;	// Font for menu header.

static RPrint		ms_printItems;				// Print used for guis.
static RPrint		ms_printHeader;			// Print used for header.

static RImage*		ms_pimBackground	= NULL;	// Dlg's background image.

static RGuiItem	ms_guiIndicator;			// Current menu item indicator.

static RTxt			ms_txtHeader;				// Header text item.

static short		ms_sCurItem			= 0;	// Current menu item index.
static short		ms_sNextMenuItem	= INVALID_MENU_ITEM;	// Item to select on next
																			// StartMenu().

static short		ms_asMenuItemPosX[MAX_MENU_ITEMS];	// Array of item X positions.
static short		ms_asMenuItemPosY[MAX_MENU_ITEMS];	// Array of item Y positions.

#ifdef MOBILE
static short		ms_asMenuItemMouseHeight[MAX_MENU_ITEMS];	// Array of item heights which can be used for mouse
#endif

static short		ms_sItemsPerColumn;			// Number of items per column

static short		ms_sCancel			= FALSE;	// TRUE to cancel current menu.

static short		ms_sItemsX			= 0;		// X position of menu items.
static short		ms_sItemsY			= 0;		// Y position of menu items.

static RResMgr*	ms_presmgr			= NULL;

static RImage		ms_imPreMenu;				// Contents of composite buffer
														// before the menu was drawn.

static short		ms_sPreMenuX;				// Location of ms_imPreMenu on screen.
static short		ms_sPreMenuY;				// Location of ms_imPreMenu on screen.

static RImage*		ms_pimComposite	= NULL;	// Composite buffer.

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Internal Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Critical callage for our msgbox.
//
//////////////////////////////////////////////////////////////////////////////
static ULONG CriticalCall(	// Returns item to select or 0.
	RMsgBox*	/*pmsgbox*/)	// MsgBox.
	{
	// For now, we expect the caller of this Menu API to call rspDoSystem(),
	// RHot::Do(), etc.

	return 0;	// No item preferred.
	}

//////////////////////////////////////////////////////////////////////////////
//
// Draws the background for ms_msgbox.
//
//////////////////////////////////////////////////////////////////////////////
static void BackCall(	// Returns nothing.
	RGuiItem* pgui,		// The RGuiItem being composed (this).
	RImage* pim,			// Image to draw into.  Try to stay within 
								// prc please.
	RRect* prc)				// Where to in image.
	{
	ASSERT(ms_pmenu != NULL);
	ASSERT(pim != NULL);
	ASSERT(prc != NULL);
	ASSERT(pgui != NULL);

	// If this is our ms_msgbox . . .
	if (pgui == &ms_msgbox)
		{
		// Fill background before any further drawing.
		rspRect(
			ms_pmenu->menuback.u32BackColor,			// Color to draw with.
			pim,												// Dst image.
			prc->sX, prc->sY, prc->sW, prc->sH);	// Dst rectangle w/i image.

		if (ms_pmenu->menuflags & MenuBackTiled)
			{
			// Determine amount to offset to center tiled area.
			short	sOffsetX	= (prc->sW % ms_pimBackground->m_sWidth) / 2;
			short	sOffsetY	= (prc->sH % ms_pimBackground->m_sHeight) / 2;
			short	sMaxX		= prc->sW - (prc->sW % ms_pimBackground->m_sWidth);
			short	sMaxY		= prc->sH - (prc->sH % ms_pimBackground->m_sHeight);
			// Tile.
			short	sX, sY;
			for (	sY = prc->sY + sOffsetY; 
					sY < sMaxY; 
					sY += ms_pimBackground->m_sHeight)
				{
				for (	sX = prc->sX + sOffsetX; 
						sX < sMaxX; 
						sX += ms_pimBackground->m_sWidth)
					{
					rspBlit(
						ms_pimBackground,		// Source image.
						pim,						// Dest image.
						0, 0,						// Source position.
						sX,						// Dest position.
						sY,
						ms_pimBackground->m_sWidth,	// Width to blit.
						ms_pimBackground->m_sHeight,	// Height to blit.
						NULL,								// Dest clip rect.
						NULL);							// Source clip rect.
					}
				}
			}
		else
			{
			// Center.
			rspBlit(
				ms_pimBackground,		// Source image.
				pim,						// Dest image.
				0, 0,						// Source position.
				prc->sX + prc->sW / 2 - ms_pimBackground->m_sWidth / 2,	// Dest position.
				prc->sY + prc->sH / 2 - ms_pimBackground->m_sHeight / 2,
				ms_pimBackground->m_sWidth,	// Width to blit.
				ms_pimBackground->m_sHeight,	// Height to blit.
				NULL,								// Dest clip rect.
				NULL);							// Source clip rect.
			}
		}
	else
		{
		// Clear background to 0 so it will be transparent when we convert it to
		// an FSPR8.
		rspRect(
			0,													// Color to draw with.
			pim,												// Dst image.
			prc->sX, prc->sY, prc->sW, prc->sH);	// Dst rectangle w/i image.
		}

	}

//////////////////////////////////////////////////////////////////////////////
//
// Copies select attributes from one gui to another.
//
//////////////////////////////////////////////////////////////////////////////
inline void SetGuiAttributes(
	RGuiItem*	pguiDst,			// Gui to set attributes to.
	RGuiItem*	pguiSrc)			// Gui to get attributes from.
	{
	pguiDst->m_u32TextColor	= pguiSrc->m_u32TextColor;
	pguiDst->m_u32BackColor	= pguiSrc->m_u32BackColor;
	pguiDst->m_pprint			= pguiSrc->m_pprint;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Moves the next item in the specified "direction".
//
//////////////////////////////////////////////////////////////////////////////
inline bool NextItem(	// Returns true, if new item selected.
	short sDirVert,		// Specifies direction (and amount) to move by being
								// negative or positive.  If 0 when sDirHorz is 0, 
								// the current item is checked and, if disabled, 
								// sDirVert is set to 1.
	short sDirHorz)		// Specifies direction (and amount) to move by being
								// negative or positive.  If 0 when sDirVert is 0, 
								// the current item is checked and, if disabled, 
								// sDirVert is set to 1.
	{
	bool	bNewSelection	= false;	// Assume no new selection.

	ASSERT(ms_pmenu != NULL);

	short	sNumItemsTraversed	= 0;
	short	sOldItem					= ms_sCurItem;

	do
		{
		// Move current item vertically.
		ms_sCurItem	+= sDirVert;
		// Move current item horizontally.
		ms_sCurItem	+= (sDirHorz * ms_sItemsPerColumn);

		// If direction is 0 . . .
		if (sDirVert == 0 && sDirHorz == 0)
			{
			// Make sDir forward 1.
			sDirVert	= 1;
			// This feature allows the caller to have the current item checked
			// and, if disabled, the next enabled item will be chosen.
			}
		else
			{
			sNumItemsTraversed++;
			}

		// Check bounds.
		if (	(ms_sCurItem > ms_sNumMenuItems)
			||	(ms_sCurItem == ms_sNumMenuItems && sDirHorz) )
			{
			if (ms_sItemsPerColumn > 0)
				{
				ms_sCurItem	= ms_sCurItem % ms_sItemsPerColumn;
				}
			else
				{
				ms_sCurItem	= 0;
				}
			}
		else if (ms_sCurItem == ms_sNumMenuItems)
			{
			ms_sCurItem	= 0;
			}
		else if (	(ms_sCurItem < -1)
					||	(ms_sCurItem == -1 && sDirHorz) )
			{
			if (ms_sItemsPerColumn > 0)
				{
#if 0
				short	sMaxMenuItem	= MAX(ms_sNumMenuItems - 1, 0);
				ms_sCurItem	= MIN(
					sMaxMenuItem, 
					short(sMaxMenuItem / ms_sItemsPerColumn * ms_sItemsPerColumn + (ms_sItemsPerColumn + ms_sCurItem) ) );
#else
				// Determine location of item in next column.
				short	sMaxMenuItem	= MAX(ms_sNumMenuItems - 1, 0);
				ms_sCurItem	= sMaxMenuItem / ms_sItemsPerColumn * ms_sItemsPerColumn + (ms_sItemsPerColumn + ms_sCurItem);
				// If too large . . .
				if (ms_sCurItem >= ms_sNumMenuItems)
					{
					ASSERT(sDirHorz < 0);
					// Go back another column.
					ms_sCurItem	+= (sDirHorz * ms_sItemsPerColumn);
					}
#endif
				}
			else
				{
				ms_sCurItem	= 0;
				}
			}
		else if (ms_sCurItem == -1)
			{
			ms_sCurItem	= MAX(ms_sNumMenuItems - 1, 0);
			}

		} while (ms_pmenu->ami[ms_sCurItem].sEnabled == FALSE
			&& sNumItemsTraversed <= ms_sNumMenuItems);

	// If we've tried all menu items . . .
	if (sNumItemsTraversed > ms_sNumMenuItems)
		{
		// Cancel this menu.
		ms_sCancel	= TRUE;
		}
	else
		{
		// If there is a callback . . .
		if (ms_pmenu->menucallbacks.fnChoice != NULL)
			{
			// If there was a change in the selected item . . .
			if (ms_sCurItem != sOldItem)
				{
				bNewSelection	= true;
				// Notify caller of change in focused item.
				(*(ms_pmenu->menucallbacks.fnChoice))(ms_pmenu, -1);
				}
			}
		}

	// Set indicator vertical position based on new item.
	// Set indicator horziontal position based on new item.
	ms_guiIndicator.Move(
		INDICATOR_POS_X(ms_sCurItem),
		INDICATOR_POS_Y(ms_sCurItem) );
	
	// Set focus to this item's gui or to no item, if none.
	ms_msgbox.SetFocus(ms_pmenu->ami[ms_sCurItem].pgui);

	return bNewSelection;
	}

#ifdef MOBILE
bool MouseChooseItem(int x, int y)
{
	for (int i=0;i < ms_sNumMenuItems; i++)
	{
		int top = ms_asMenuItemPosY[i] - ms_asMenuItemMouseHeight[i] / 2;
		int bottom =  ms_asMenuItemPosY[i] + ms_asMenuItemMouseHeight[i] / 2;
		TRACE("MouseChooseItem %d  top=%d  bot=%d",i,top,bottom);
		if ((y >= top) && (y < bottom)) //Check if Y is in a controls area
		{
			if (ms_pmenu->ami[i].sEnabled) //Check item is actually enabled
			{
				ms_sCurItem = i;
				(*(ms_pmenu->menucallbacks.fnChoice))(ms_pmenu, -1);
				// Set focus to this item's gui or to no item, if none.
				ms_msgbox.SetFocus(ms_pmenu->ami[ms_sCurItem].pgui);

				return true;
			}
		}
	}
	return false;
}
#endif
//////////////////////////////////////////////////////////////////////////////
// External Functions.
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
// Sets up the menu described to be run via DoMenu*() calls.
//
//////////////////////////////////////////////////////////////////////////////
extern short StartMenu(				// Returns 0 on success.
	Menu*	pmenu,						// In:  Pointer to Menu describing menu.
	RResMgr* presmgr,					// In:  Resource manager ptr.
	RImage*	pimDst)					// In:  Src for erase data.  Erase data is
											// updated back over the menu on StopMenu()
											// calls.
	{
	short	sRes	= 0;	// Assume success.

	// End any existing menu.
	StopMenu();

	// If there is a new menu . . .
	if (pmenu != NULL)
		{
        rspKeyRepeat(true);

		U32	u32TextColor;
		U32	u32TextShadowColor;
		short	sMapStartIndex;
		short	sMapNumEntries;

		// Get composite buffer.
		ms_pimComposite	= pimDst;

		// Set new menu.
		ms_pmenu		= pmenu;

		ms_sCancel	= FALSE;

		// Get info on composite buffer.
		RImage*	pimComposite	= NULL;

		rspNameBuffers(&pimComposite);

		ASSERT(pimComposite != NULL);

		// If there is a callback . . .
		if (ms_pmenu->menucallbacks.fnInit != NULL)
			{
			if ((*(ms_pmenu->menucallbacks.fnInit))(ms_pmenu, TRUE) == 0)
				{
				}
			else
				{
				TRACE("StartMenu(): StartMenu() aborted by callback.\n");
				// Abort right away.
				ms_sCancel	= TRUE;
				}
			}

		ASSERT(presmgr != NULL);
		// Store pointer to desired res mgr.
		ms_presmgr	= presmgr;

		// Determine number of menu items.
		ms_sNumMenuItems	= 0;
		while (pmenu->ami[ms_sNumMenuItems].pszText != NULL)
			{
			ms_sNumMenuItems++;
			}

		ms_sNumMenuItems	= MIN(ms_sNumMenuItems, MAX_MENU_ITEMS);

		// Set GUI settings.
		ms_msgbox.m_sTransparent	= ms_pmenu->menugui.sTransparent;

		// If there is a desired font for menu header . . .
		if (ms_pmenu->menuheader.pszFontFile != NULL)
			{
			// Attempt to allocate get font resource . . .
			if (rspGetResource(
				ms_presmgr,
				ms_pmenu->menuheader.pszFontFile,
				&ms_pfontHeader) == 0)
				{
				// Successfully loaded font.
				ms_printHeader.SetFont(ms_pmenu->menuheader.sHeight, ms_pfontHeader);
				}
			else
				{
				TRACE("StartMenu(): Unable to load font file \"%s\" via ms_presmgr->\n",
					ms_pmenu->menuheader.pszFontFile);
				sRes	= -2;

				goto Done;
				}
			}
		else
			{
			// If there is no font specified in ms_pmenu, we must use the one
			// currently being used by the general GUI items.

			// In order to use the default font, one must exist!
			// If no font has been set in RGuiItem::ms_printItems, we're done.
			if (RGuiItem::ms_print.GetFont() != NULL)
				{
				// Use current font from default RPrint used by all GUI items.
				ms_printHeader.SetFont(ms_pmenu->menuheader.sHeight, RGuiItem::ms_print.GetFont());
				}
			else
				{
				TRACE("StartMenu(): Menu does not specify a font and neither does "
					"RGuiItem::ms_print.\n");
				sRes	= -3;

				goto Done;
				}
			}

		if (ms_pmenu->menuflags & MenuHeaderTextShadow)
			{
			ms_txtHeader.m_sTextShadowOffsetX	= SHADOW_X_PIXELS;
			ms_txtHeader.m_sTextShadowOffsetY	= SHADOW_Y_PIXELS;
			ms_txtHeader.m_sTextEffects			= RGuiItem::Shadow;
			}
		else
			{
			ms_txtHeader.m_sTextEffects			= 0;
			}

		// If there is a desired font for menu items . . .
		if (ms_pmenu->menuitemsfont.pszFile != NULL)
			{
			// Attempt to allocate get font resource . . .
			if (rspGetResource(
				ms_presmgr,
				ms_pmenu->menuitemsfont.pszFile,
				&ms_pfontItems) == 0)
				{
				// Successfully loaded font.
				ms_printItems.SetFont(ms_pmenu->menuitemsfont.sHeight, ms_pfontItems);
				}
			else
				{
				TRACE("StartMenu(): Unable to load font file \"%s\" via ms_presmgr->\n",
					ms_pmenu->menuitemsfont.pszFile);
				sRes	= -2;

				goto Done;
				}
			}
		else
			{
			// If there is no font specified in ms_pmenu, we must use the one
			// currently being used by the general GUI items.

			// In order to use the default font, one must exist!
			// If no font has been set in RGuiItem::ms_printItems, we're done.
			if (RGuiItem::ms_print.GetFont() != NULL)
				{
				// Use current font from default RPrint used by all GUI items.
				ms_printItems.SetFont(ms_pmenu->menuitemsfont.sHeight, RGuiItem::ms_print.GetFont());
				}
			else
				{
				TRACE("StartMenu(): Menu does not specify a font and neither does "
					"RGuiItem::ms_print.\n");
				sRes	= -3;

				goto Done;
				}
			}

		// Set dlg's attributes based on ms_pmenu.
		// Set dlg's attributes that are not based on ms_pmenu.
		ms_msgbox.m_sBorderThickness	= 0;	// No border.
		// Make sure dlg's background is drawn by this module.
		ms_msgbox.m_backcall				= BackCall;
		// Use our print.
		ms_msgbox.m_pprint				= &ms_printItems;
		// Use this text height.
		ms_msgbox.m_sFontCellHeight	= ms_pmenu->menuitemsfont.sHeight;

		// Set callback for iterative calls.
		ms_msgbox.m_mbcUser				= CriticalCall;
	
		// Compute position of dialog.
		short	sX;
		short	sY;
		short sW;
		short	sH;

		if (ms_pmenu->menuflags & MenuPosCenter)
			{
			// Centered menu.
			sW	= ms_pmenu->menupos.sW;
			sH	= ms_pmenu->menupos.sH;
			sX	= pimComposite->m_sWidth / 2 - sW / 2;
			sY	= pimComposite->m_sHeight / 2 - sH / 2;
			}
		else
			{
			if (ms_pmenu->menuflags & MenuPosFullscreen)
				{
				// Full screen menu.
				sX	= 0;
				sY	= 0;
				sW	= pimComposite->m_sWidth;
				sH	= pimComposite->m_sHeight;
				}
			else
				{
				// Menu at specified location.
				sX	= ms_pmenu->menupos.sX;
				sY	= ms_pmenu->menupos.sY;
				sW	= ms_pmenu->menupos.sW;
				sH	= ms_pmenu->menupos.sH;
				}
			}

		// If there is a background . . .
		if (ms_pmenu->menuback.pszFile != NULL)
			{
			// Load background . . .
			if (rspGetResource(
				ms_presmgr,
				ms_pmenu->menuback.pszFile,
				&ms_pimBackground) == 0)
				{
				// Successfully loaded dlg background.
				// If paletted . . .
				if (ms_pimBackground->m_pPalette != NULL)
					{
					short	sEntrySize	= ms_pimBackground->m_pPalette->m_sPalEntrySize;
					short	sStartIndex	= ms_pmenu->menuback.sSetStartIndex;

					// Set palette.
					rspSetPaletteEntries(
						sStartIndex,
						ms_pmenu->menuback.sSetNumEntries,
						ms_pimBackground->m_pPalette->Red(sStartIndex),
						ms_pimBackground->m_pPalette->Green(sStartIndex),
						ms_pimBackground->m_pPalette->Blue(sStartIndex),
						sEntrySize);

					// Update hardware.
					rspUpdatePalette();
					}
				}
			else
				{
				TRACE("StartMenu(): Unable to load \"%s\" via ms_presmgr.\n", 
					ms_pmenu->menuback.pszFile);
				sRes	= -6;

				goto Done;
				}
			}

		// Get color indices.
		U8	au8Red[256];
		U8	au8Green[256];
		U8	au8Blue[256];
		rspGetPaletteEntries(
			0,				// Palette entry to start copying to (has no effect on source!)
			256,			// Number of palette entries to do
			au8Red,		// Pointer to first red component to copy to
			au8Green,	// Pointer to first green component to copy to
			au8Blue,		// Pointer to first blue component to copy to
			sizeof(U8)	// Number of bytes by which to increment pointers after each copy
			);

		sMapStartIndex	= ms_pmenu->menuback.sMapStartIndex;
		sMapNumEntries	= ms_pmenu->menuback.sMapNumEntries;
		if (sMapNumEntries == 0)
			{
			sMapNumEntries	= 256;
			}

		u32TextColor
			= rspMatchColorRGB(											// Out: Matched index.
				RED(ms_pmenu->menuitemsfont.u32ForeColor),		// In:  Pixel's red value.                                 
				GREEN(ms_pmenu->menuitemsfont.u32ForeColor),		// In:  Pixel's green value.                               
				BLUE(ms_pmenu->menuitemsfont.u32ForeColor),		// In:  Pixel's blue value.                                
				sMapStartIndex,											// In:  Min mappable index (affects source).               
				sMapNumEntries,											// In:  Num mappable indices (affects source).             
				au8Red,														// In:  Beginning of red color table.                      
				au8Green,													// In:  Beginning of green color table.                    
				au8Blue,														// In:  Beginning of blue color table.                     
				sizeof(U8)													// In:  Size to increment between each index in each table.
				);

		u32TextShadowColor
			= rspMatchColorRGB(											// Out: Matched index.
				RED(ms_pmenu->menuitemsfont.u32ShadowColor),		// In:  Pixel's red value.                                 
				GREEN(ms_pmenu->menuitemsfont.u32ShadowColor),	// In:  Pixel's green value.                               
				BLUE(ms_pmenu->menuitemsfont.u32ShadowColor),	// In:  Pixel's blue value.                                
				sMapStartIndex,											// In:  Min mappable index (affects source).               
				sMapNumEntries,											// In:  Num mappable indices (affects source).             
				au8Red,														// In:  Beginning of red color table.                      
				au8Green,													// In:  Beginning of green color table.                    
				au8Blue,														// In:  Beginning of blue color table.                     
				sizeof(U8)													// In:  Size to increment between each index in each table.
				);

		ms_msgbox.m_u32BackColor		= 0;

		ms_txtHeader.m_u32TextColor
			= rspMatchColorRGB(										// Out: Matched index.
				RED(ms_pmenu->menuheader.u32ForeColor),		// In:  Pixel's red value.                                 
				GREEN(ms_pmenu->menuheader.u32ForeColor),		// In:  Pixel's green value.                               
				BLUE(ms_pmenu->menuheader.u32ForeColor),		// In:  Pixel's blue value.                                
				sMapStartIndex,										// In:  Min mappable index (affects source).               
				sMapNumEntries,										// In:  Num mappable indices (affects source).             
				au8Red,													// In:  Beginning of red color table.                      
				au8Green,												// In:  Beginning of green color table.                    
				au8Blue,													// In:  Beginning of blue color table.                     
				sizeof(U8)												// In:  Size to increment between each index in each table.
				);

		ms_txtHeader.m_u32TextShadowColor
			= rspMatchColorRGB(										// Out: Matched index.
				RED(ms_pmenu->menuheader.u32ShadowColor),		// In:  Pixel's red value.                                 
				GREEN(ms_pmenu->menuheader.u32ShadowColor),	// In:  Pixel's green value.                               
				BLUE(ms_pmenu->menuheader.u32ShadowColor),	// In:  Pixel's blue value.                                
				sMapStartIndex,										// In:  Min mappable index (affects source).               
				sMapNumEntries,										// In:  Num mappable indices (affects source).             
				au8Red,													// In:  Beginning of red color table.                      
				au8Green,												// In:  Beginning of green color table.                    
				au8Blue,													// In:  Beginning of blue color table.                     
				sizeof(U8)												// In:  Size to increment between each index in each table.
				);

		ms_txtHeader.m_u32BackColor		= 0;

		// Create dialog . . .
		if (ms_msgbox.Create(sX, sY, sW, sH, pimComposite->m_sDepth) == 0)
			{
			// Successfully created dialog.
			}
		else
			{
			TRACE("StartMenu(): Unable to create ms_msgbox.\n");
			sRes	= -5;

			goto Done;
			}

		ms_sItemsX	= ms_pmenu->menupos.sItemX;
		ms_sItemsY	= ms_pmenu->menupos.sItemY;

		// If negative . . .
		if (ms_sItemsX < 0)
			{
			// Offset from center.
			ms_sItemsX	+= ms_msgbox.m_im.m_sWidth / 2;
			}

		// If negative . . .
		if (ms_sItemsY < 0)
			{
			// Offset from center.
			ms_sItemsY	+=	ms_msgbox.m_im.m_sHeight / 2;
			}

		// If there is menu header text . . .
		if (ms_pmenu->menuheader.pszHeaderText[0] != '\0')
			{
			short	sPosX	= ms_pmenu->menupos.sHeaderX;
			short	sPosY	= ms_pmenu->menupos.sHeaderY;

			if (sPosX < 0)
				{
				// Offset from center.
				sPosX	+=	ms_msgbox.m_im.m_sWidth / 2;
				}

			if (sPosY < 0)
				{
				// Offset from center.
				sPosY	+=	ms_msgbox.m_im.m_sHeight / 2;
				}

			ms_txtHeader.m_pprint					= &ms_printHeader;
			ms_txtHeader.m_sBorderThickness		= 0;
			ms_txtHeader.m_sFontCellHeight		= ms_pmenu->menuheader.sHeight;

			// Determine width for header . . .
			// Store text width.
			short sTextWidth	= ms_txtHeader.m_pprint->GetWidth(ms_pmenu->menuheader.pszHeaderText);
			// Active when visible.
			ms_txtHeader.m_sActive	= TRUE;
			ms_txtHeader.SetVisible(TRUE);

			// If centering specified . . .
			if (ms_pmenu->menuflags & MenuHeaderTextCenter)
				{
				sPosX	= ms_msgbox.m_im.m_sWidth / 2 - sTextWidth / 2;
				}
			
			// Update header item.
			ms_txtHeader.SetText(ms_pmenu->menuheader.pszHeaderText);
			ms_txtHeader.m_sX						= sPosX;
			ms_txtHeader.m_sY						= sPosY;

			// Reserve space for borders, if there are any.
			short sTotalBorderThickness	= ms_txtHeader.GetTopLeftBorderThickness()
													+ ms_txtHeader.GetBottomRightBorderThickness();

			// Create item . . .
			if (ms_txtHeader.Create(
				sPosX, sPosY,
				sTextWidth + sTotalBorderThickness + SHADOW_X_PIXELS,
				ms_txtHeader.m_sFontCellHeight + sTotalBorderThickness + SHADOW_Y_PIXELS,
				ms_msgbox.m_im.m_sDepth) == 0)
				{
				// Successfully created header item.
				ms_txtHeader.m_sTransparent			= TRUE;
				ms_txtHeader.m_u32TransparentColor	= 0;
				// Add to msg box . . .
				ms_txtHeader.SetParent(&ms_msgbox);
				}
			else
				{
				TRACE("StartMenu(): ms_txtHeader.Create() failed.\n");
				sRes	= -11;

				goto Done;
				}
			}

		ms_sItemsPerColumn	= 0;

		if (ms_sNumMenuItems > 0)
			{
			short	sMaxItemX	= ms_pmenu->menupos.sMaxItemX;
			short	sMaxItemY	= ms_pmenu->menupos.sMaxItemY;

			// If less than 1 . . .
			if (sMaxItemX < 1)
				{
				sMaxItemX	+= ms_msgbox.m_im.m_sWidth;
				}

			// If less than 1 . . .
			if (sMaxItemY < 1)
				{
				sMaxItemY	+= ms_msgbox.m_im.m_sHeight;
				}

			short	sPosX				= ms_sItemsX;
			short sIndex			= 0;

			while (sIndex < ms_sNumMenuItems && sPosX < sMaxItemX)
				{
				short	sPosY					= ms_sItemsY;
				short	sMaxGuiPosX			= sPosX;
				short	sMaxItemExtentX	= sPosX;
				short	sStartIndex			= sIndex;	// First index this column.

				RTxt*	ptxt;
				for ( ; sIndex < ms_sNumMenuItems; sIndex++)
					{
					// Color is based on whether the item is enabled or not.
					if (ms_pmenu->ami[sIndex].sEnabled)
						{
						ms_msgbox.m_u32TextColor			= u32TextColor;
						ms_msgbox.m_u32TextShadowColor	= u32TextShadowColor;
						}
					else
						{
						ms_msgbox.m_u32TextColor			= u32TextShadowColor;
						ms_msgbox.m_u32TextShadowColor	= u32TextColor;
						}

					// Add a text item and get a pointer to it.
					// Note that the item is stored and maintained by RMsgBox.
					ptxt	= ms_msgbox.AddText(
						ms_pmenu->ami[sIndex].pszText,
						sPosX, sPosY,
						sIndex,
						SHADOW_X_PIXELS,
						SHADOW_Y_PIXELS);

					if (ptxt != NULL)
						{
						// If shadow specified . . .
						if (ms_pmenu->menuflags & MenuItemTextShadow)
							{
							ptxt->m_sTextShadowOffsetX	= SHADOW_X_PIXELS;
							ptxt->m_sTextShadowOffsetY	= SHADOW_Y_PIXELS;
							ptxt->m_sTextEffects			= RGuiItem::Shadow;
							// Compose with shadow.
							ptxt->Compose();
							}

						// Remember pos of vertical center of this item.
						ms_asMenuItemPosY[sIndex]	= sPosY + ptxt->m_im.m_sHeight / 2;
						// Remember x position of this item.
						ms_asMenuItemPosX[sIndex]	= sPosX;
#ifdef MOBILE
						// Remember height of this item. Height of item + spacing between them
						ms_asMenuItemMouseHeight[sIndex]	= ptxt->m_im.m_sHeight + ms_pmenu->menupos.sItemSpacingY;
#endif
						// If there is a gui . . .
						if (ms_pmenu->ami[sIndex].pgui != NULL)
							{
							// Put gui item just to right of text and
							// centered vertically with text.
							ms_pmenu->ami[sIndex].pgui->Move(
								sPosX + ptxt->m_im.m_sWidth + ms_pmenu->menupos.sIndicatorSpacingX,
								sPosY + ptxt->m_im.m_sHeight / 2 - ms_pmenu->ami[sIndex].pgui->m_im.m_sHeight / 2);

							// Remember the position of the furthest GUI.
							sMaxGuiPosX	= MAX(sMaxGuiPosX, ms_pmenu->ami[sIndex].pgui->m_sX);

							ms_pmenu->ami[sIndex].pgui->SetParent(&ms_msgbox);
							ms_pmenu->ami[sIndex].pgui->SetVisible(TRUE);

							// Remember the extent of the furthest item.
							sMaxItemExtentX	= MAX(
								sMaxItemExtentX, 
								short(ms_pmenu->ami[sIndex].pgui->m_sX + ms_pmenu->ami[sIndex].pgui->m_im.m_sWidth) );
							}
						else
							{
							// Remember the extent of the furthest item.
							sMaxItemExtentX	= MAX(
								sMaxItemExtentX, 
								short(ptxt->m_sX + ptxt->m_im.m_sWidth) );
							}

						ptxt->m_sTransparent				= TRUE;
						ptxt->m_u32TransparentColor	= 0;

						// Move to pos for next item.
						sPosY += ptxt->m_im.m_sHeight + ms_pmenu->menupos.sItemSpacingY;

						// If this next position is too far . . .
						if (sPosY + ptxt->m_im.m_sHeight > sMaxItemY)
							{
							sIndex++;
							break;
							}
						}
					else
						{
						TRACE("StartMenu(): ms_msgbox.AddText() failed for item %s.\n", 
							ms_pmenu->ami[sIndex].pszText);
						sRes	= -10;

						goto Done;
						}
					}

				if (ms_sItemsPerColumn == 0)
					{
					ms_sItemsPerColumn	= sIndex;
					}

				// If columnize GUIs specified . . .
				if (ms_pmenu->menuflags & MenuColumnizeGuis)
					{
					// End index EXCLUSIVE is either an entire column
					// or a partial (for the last column).
					short	sEndIndex	= MIN(sIndex, short(sStartIndex + ms_sItemsPerColumn) );

					for (sIndex = sStartIndex; sIndex < sEndIndex; sIndex++)
						{
						// If there is a gui . . .
						if (ms_pmenu->ami[sIndex].pgui != NULL)
							{
							// Shift GUI over to furthest GUI position.
							ms_pmenu->ami[sIndex].pgui->Move(
								sMaxGuiPosX,
								ms_pmenu->ami[sIndex].pgui->m_sY);
							}
						}
					}

				// Next column.
				sPosX	= sMaxItemExtentX + ms_pmenu->menupos.sIndicatorSpacingX + ms_guiIndicator.m_im.m_sWidth;
				}

			// Store number of menu items that fit.
			ms_sNumMenuItems	= sIndex;
			}

		// If we went back to this menu . . .
		if (ms_sNextMenuItem != INVALID_MENU_ITEM)
			{
			// We don't need to use the ITEMINDEX macro if we set this
			// value, but just in case a callback decides to use the
			// extended indexing . . ..
			ms_sCurItem	= ITEMINDEX(ms_sNextMenuItem);
			// Clear back item.
			ms_sNextMenuItem	= INVALID_MENU_ITEM;
			}
		else
			{
			// Start at default item.
			ms_sCurItem	= ITEMINDEX(ms_pmenu->menuautoitems.sDefaultItem);
			}

		// If there is a menu indicator . . .
		if (ms_pmenu->menuindicator.pszFile != NULL)
			{
			// Here we would like to load the resource right into ms_guiIndicator.m_im, but
			// cannot since that is already instantiated and the res manager takes care of
			// instantiation of resources.  So, we must get the resource, allocate
			// ms_guiIndicator.m_im such that the indicator image will fit, and then blit
			// it in from the resource.
			RImage*	pimIndicator;
			if (rspGetResource(
				ms_presmgr,
				ms_pmenu->menuindicator.pszFile, 
				&pimIndicator) == 0)
				{
				// Successfully loaded image.

				// Allocate indicator such that it can contain the image . . .
				if (ms_guiIndicator.m_im.CreateImage(
					pimIndicator->m_sWidth,
					pimIndicator->m_sHeight,
					RImage::BMP8
					) == 0)
					{
					// Blt into indicator.
					rspBlit(
						pimIndicator,
						&(ms_guiIndicator.m_im),
						0, 0,
						0, 0,
						pimIndicator->m_sWidth,
						pimIndicator->m_sHeight);

					// Make indicator a child of ms_msgbox.
					ms_guiIndicator.SetParent(&ms_msgbox);
					
					if (ms_pmenu->menuindicator.type != RImage::NOT_SUPPORTED)
						{
						if (ms_guiIndicator.m_im.Convert(ms_pmenu->menuindicator.type) == ms_pmenu->menuindicator.type)
							{
							// Successfully converted image.
							}
						else
							{
							TRACE("StartMenu(): Unable to convert indicator to desired image type (%s).\n",
								RImage::ms_astrTypeNames[ms_pmenu->menuindicator.type]);
							sRes	= -8;

							goto Done;
							}

						// Set indicator position.
						ms_guiIndicator.m_sX	= ms_sItemsX - ms_guiIndicator.m_im.m_sWidth
														- ms_pmenu->menupos.sIndicatorSpacingX;

						// This will set ms_guiIndicator.m_sY and will skip to the first enabled entry
						// which starting with ms_sCurItem.
						NextItem(0, 0);
						}
					}
				else
					{
					TRACE("StartMenu(): Unable to load \"%s\".\n", ms_pmenu->menuindicator.pszFile);
					sRes	= -7;

					goto Done;
					}

				// Release right away.
				rspReleaseResource(ms_presmgr, &pimIndicator);
				}
			else
				{
				TRACE("StartMenu(): Unabled to load \"%s\".\n", ms_pmenu->menuindicator.pszFile);
				sRes	= -7;

				goto Done;
				}
			}
		else
			{
			// Make indicator top level (so it won't show up when ms_msgbox is drawn).
			// Note we could make the indicator not visible, instead.
			ms_guiIndicator.SetParent(NULL);
			}

		// If successful . . .
		if (sRes == 0 && ms_pimComposite)
			{
			// Destory any existing contents.
			ms_imPreMenu.DestroyData();

			// Create a space to store the current composite buffer's contents . . .
			if (ms_imPreMenu.CreateImage(
				ms_msgbox.m_im.m_sWidth,
				ms_msgbox.m_im.m_sHeight,
				RImage::BMP8) == 0)
				{
				// We must lock the composite buffer before reading from it.
				rspLockBuffer();

				// Store screen contents.
				rspBlit(
					ms_pimComposite,				// Src.
					&ms_imPreMenu,					// Dst.
					ms_msgbox.m_sX,				// Src x.
					ms_msgbox.m_sY,				// Src y.
					0,									// Dst x.
					0,									// Dst y.
					ms_msgbox.m_im.m_sWidth,	// Both.
					ms_msgbox.m_im.m_sHeight,	// Both.
					NULL,								// Dst clip.
					NULL);							// Src clip.

				// Unlock, we're done.
				rspUnlockBuffer();
				}
			else
				{
				TRACE("StartMenu(): ms_imPreMenu.CreateImage() failed.\n");
				sRes	= -20;

				goto Done;
				}
			}

Done:
		// If any errors occurred . . .
		if (sRes != 0)
			{
			StopMenu();

			TRACE("StartMenu(): Menu not started due to start up error.\n");
			}
		else
			{
			// Show and activate msg box and children.
			ms_msgbox.SetVisible(TRUE);
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Performs one iteration of menu input logic.  Call this repeatedly in
// conjunction with DoMenuOutput() to run the menu.
//
//////////////////////////////////////////////////////////////////////////////
extern void DoMenuInput(		// Returns nothing.
	RInputEvent* pie,				// In:  Most recent user input event.
	short UseJoystick)
										// Out: pie->sUsed = TRUE, if used.
	{
	// If there is a current menu . . .
	if (ms_pmenu != NULL)
		{
		short	sChooseCurrent	= FALSE;

		// If there is an unused key event . . .
		if (pie->sUsed == FALSE && pie->type == RInputEvent::Key)
			{
			switch (pie->lKey)
				{
				case RSP_GK_DOWN:
				case '\t':
				case RSP_GKF_CONTROL | '\t':
					if (NextItem(1, 0) == true)
						{
						// Absorb key.
						pie->sUsed	= TRUE;
						}

					break;

				case RSP_GK_UP:
				case RSP_GKF_SHIFT | '\t':
				case RSP_GKF_SHIFT | RSP_GKF_CONTROL | '\t':
					if (NextItem(-1, 0) == true)
						{
						// Absorb key.
						pie->sUsed	= TRUE;
						}

					break;

				case RSP_GK_LEFT:
					if (NextItem(0, -1) == true)
						{
						// Absorb key.
						pie->sUsed	= TRUE;
						}

					break;

				case RSP_GK_RIGHT:
					if (NextItem(0, 1) == true)
						{
						// Absorb key.
						pie->sUsed	= TRUE;
						}

					break;

				case 27:
					ms_sCancel	= TRUE;

					// Absorb key.
					pie->sUsed	= TRUE;

					break;

				case '\r':
					sChooseCurrent	= TRUE;

					// Absorb key.
					pie->sUsed	= TRUE;

					break;
				}
			}

		if (UseJoystick)
		{
			XInputState xis = {};	// Current XInput state			
			GetXInputState(&xis);

			// Menu up.
			if (xis.ButtonState[XINPUT_BUTTON_MENUUP] == XInputState::Press
				|| xis.AxisState[XINPUT_AXIS_MENUUP] == XInputState::Press)
				NextItem(-1, 0);

			// Menu down.
			if (xis.ButtonState[XINPUT_BUTTON_MENUDOWN] == XInputState::Press
				|| xis.AxisState[XINPUT_AXIS_MENUDOWN] == XInputState::Press)
				NextItem(1, 0);

			// Menu left.
			if (xis.ButtonState[XINPUT_BUTTON_MENULEFT] == XInputState::Press
				|| xis.AxisState[XINPUT_AXIS_MENULEFT] == XInputState::Press)
			{
				NextItem(0, -1);
				//!! HACK
				pie->lKey = RSP_GK_LEFT;
				pie->sUsed = 0;
				pie->type = RInputEvent::Key;
			}

			// Menu right.
			if (xis.ButtonState[XINPUT_BUTTON_MENURIGHT] == XInputState::Press
				|| xis.AxisState[XINPUT_AXIS_MENURIGHT] == XInputState::Press)
			{
				NextItem(0, 1);
				//!! HACK
				pie->lKey = RSP_GK_RIGHT;
				pie->sUsed = 0;
				pie->type = RInputEvent::Key;
			}

			// Cancel/goback.
			if (xis.ButtonState[XINPUT_BUTTON_BACK] == XInputState::Press
				|| xis.ButtonState[XINPUT_BUTTON_BACK2] == XInputState::Press)
				ms_sCancel = TRUE;

			// Confirm.
			if (xis.ButtonState[XINPUT_BUTTON_CONFIRM] == XInputState::Press
				|| xis.ButtonState[XINPUT_BUTTON_START] == XInputState::Press)
				sChooseCurrent = TRUE;
		}

#ifdef MOBILE
		if (pie->sUsed == FALSE && pie->type == RInputEvent::Mouse)
		{
			//TRACE("Mouse menu %d  %d  %d", pie->sPosX , pie->sPosY, pie->sButtons);
			//TRACE("Mouse menu POS %d  %d  %d", ms_pmenu->menupos.sX,ms_pmenu->menupos.sY   ,ms_pmenu->menupos.sItemY);
			//TRACE("Mousemsgbox %d  %d",ms_msgbox.m_sX,ms_msgbox.m_sY);

			if ( pie->sButtons)
			{
				//Don't forget to subtract the messagebox position
				if (MouseChooseItem(pie->sPosX - ms_msgbox.m_sX, pie->sPosY -  ms_msgbox.m_sY))
				{
					sChooseCurrent	= TRUE;
					pie->sUsed	= TRUE;
				}
			}
			//pie->sUsed	= TRUE;
		}
#endif

		// If cancelled . . .
		if (ms_sCancel != FALSE)
			{
			short	sCancelItem	= ITEMINDEX(ms_pmenu->menuautoitems.sCancelItem);
			// If enabled . . .
//			if (ms_pmenu->ami[sCancelItem].sEnabled != FALSE)
				{
				// Choose cancel item.
				ms_sCurItem		= sCancelItem;
				sChooseCurrent	= TRUE;
				}

			// Reset cancel flag.
			ms_sCancel		= FALSE;
			}

		if (sChooseCurrent != FALSE)
			{
			bool	bAcceptChoice	= true;

			// If there is a callback . . .
			if (ms_pmenu->menucallbacks.fnChoice != NULL)
				{
				// See if callback will allow this choice . . .
				bAcceptChoice = (*(ms_pmenu->menucallbacks.fnChoice))(ms_pmenu, ms_sCurItem);
				}

			// If choice is still okay . . .
			if (bAcceptChoice == true)
				{
				// If there is still a current menu . . .
				if (ms_pmenu != NULL)
					{
					Menu*	pmenuNext	= ms_pmenu->ami[ms_sCurItem].pmenu;
					// If this is the cancel item . . .
					if (ms_sCurItem == ITEMINDEX(ms_pmenu->menuautoitems.sCancelItem))
						{
						// If there is a go back menu . . .
						if (ms_pmenu->menuautoitems.pmenuBack != NULL)
							{
							// Start menu to go back to.
							pmenuNext			= ms_pmenu->menuautoitems.pmenuBack;
							// Should we clear the go back menu item (pmenuBack)
							// here? ***
							// Set selection for new menu.
							ms_sNextMenuItem	= ms_pmenu->menuautoitems.sBackItem;
							}
						}
					else
						{
						if (pmenuNext != NULL)
							{
							// Store menu to go back to.
							pmenuNext->menuautoitems.pmenuBack	= ms_pmenu;
							// Store item to go back to.
							pmenuNext->menuautoitems.sBackItem	= ms_sCurItem;
							}
						}

					// If there is a menu . . .
					if (pmenuNext != NULL)
						{
						// Start the new menu.
						StartMenu(pmenuNext, ms_presmgr, ms_pimComposite);
						}
					}
				}
			}

		// Make sure menu is still active (even after callbacks) . . .
		if (ms_pmenu != NULL)
			{
			// Call msgbox.
			ms_msgbox.DoModeless(pie);
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Performs one iteration of menu output logic.  Call this repeatedly in
// conjunction with DoMenuInput() to run the menu.
//
//////////////////////////////////////////////////////////////////////////////
extern void DoMenuOutput(	// Returns nothing.
	RImage*	pimDst)			// In:  Destination image for menu BLiTs.
	{
	// If there is a current menu . . .
	if (ms_pmenu != NULL)
		{
		// Draw msgbox.
		ms_msgbox.Draw(pimDst);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the current menu.
//
//////////////////////////////////////////////////////////////////////////////
extern Menu* GetCurrentMenu(void)	// Returns the a pointer to the current
												// menu or NULL if there is none.
	{
	return ms_pmenu;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Call this to release all memory being used by the menu system
// It should be safe to call this function even if StartMenu()  
// failed, and it should be safe to call it multiple times -- in
// both cases, it shouldn't do anything.                        
//
//////////////////////////////////////////////////////////////////////////////
extern short StopMenu(void)		// Returns 0 on success.
	{
	short	sRes	= 0;	// Assume success.

	// If we have a current menu . . .
	if (ms_pmenu != NULL)
		{
		// Clean up.

		// Destroy any dynamic dlg data.
		ms_msgbox.Destroy();

		// Remove all child items created via ms_msgbox.
		ms_msgbox.RemoveAll();

		// Hide and deactivate msg box and remaining children.
		ms_msgbox.SetVisible(FALSE);

		short	sIndex;
		for (sIndex	= 0; sIndex < ms_sNumMenuItems; sIndex++)
			{
			// If there is a gui . . .
			if (ms_pmenu->ami[sIndex].pgui != NULL)
				{
				ms_pmenu->ami[sIndex].pgui->SetParent(NULL);
				}
			}

		// Destroy background data.
		if (ms_pimBackground != NULL)
			{
			ms_presmgr->Release(ms_pimBackground);
			ms_pimBackground	= NULL;
			}

		// Destroy indicator data.
		ms_guiIndicator.Destroy();

		// Delete fonts.
		if (ms_pfontItems != NULL)
			{
			ms_presmgr->Release(ms_pfontItems);
			ms_pfontItems	= NULL;
			}

		if (ms_pfontHeader != NULL)
			{
			ms_presmgr->Release(ms_pfontHeader);
			ms_pfontHeader	= NULL;
			}

		ms_sNumMenuItems	= 0;

		// If there's a callback . . .
		if (ms_pmenu->menucallbacks.fnInit != NULL)
			{
			// Let the callback know we're cleaning up.
			(*ms_pmenu->menucallbacks.fnInit)(ms_pmenu, FALSE);
			}

		// Clear current menu pointer.
		ms_pmenu	= NULL;

		if (ms_pimComposite != NULL)
			{
			// We must lock the composite buffer before writing to it.
			rspLockBuffer();

			// Restore screen contents.
			rspBlit(
				&ms_imPreMenu,					// Src.
				ms_pimComposite,				// Dst.
				0,									// Src x.
				0,									// Src y.
				ms_msgbox.m_sX,				// Dst x.
				ms_msgbox.m_sY,				// Dst y.
				ms_msgbox.m_im.m_sWidth,	// Both.
				ms_msgbox.m_im.m_sHeight,	// Both.
				NULL,								// Dst clip.
				NULL);							// Src clip.


			// Unlock, we're done.
			rspUnlockBuffer();

			// Destory contents.
			ms_imPreMenu.DestroyData();

			// Clear pointer.
			ms_pimComposite	= NULL;
			}

            rspKeyRepeat(false);
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the Menu's background image.
//
//////////////////////////////////////////////////////////////////////////////
extern RImage* GetCurrentMenuBackground(void)	// Returns a pointer to the
																// current background image
																// or NULL, if none.
	{
	return ms_pimBackground;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the Menu's GUI.
//
//////////////////////////////////////////////////////////////////////////////
extern RGuiItem* GetCurrentMenuBox(void)	// Returns a pointer to the current
														// menu GUI or NULL, if none.
	{
	return (RGuiItem*)&ms_msgbox;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
