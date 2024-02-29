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
//////////////////////////////////////////////////////////////////////////////
//
// GuiItem.CPP
// 
// History:
//		08/07/96 JMI	Started.
//
//		08/11/96	JMI	When m_sBorderThickness was 0, GetClient was returning
//							0 for the width and height by mistake.  This was 
//							remedied(sp?) such that it now uses the width and
//							height of the image.
//
//		08/12/96	JMI	Added m_sJustification to allow the user to specify text
//							justification and DrawText so derived classes can
//							utilize this functionality.
//
//		08/15/96	JMI	Blit() was checking dest image type for BLiT type instead 
//							of source image type.
//
//		08/15/96	JMI	Added TopPosToChild() complement to ChildPosToTop().
//
//		08/20/96	JMI	SetParent now calls OffsetTopLevelPos to update top level
//							items.
//
//		08/20/96	JMI	~CGuiItem() now releases its parent and all its children.
//
//		09/24/96	JMI	Borders are now not symmetrical.  This makes the edges
//							appear more 3D like and causes a button's client to appear
//							to sink if the button has borders.  This means that there
//							now has to be a GetTopLeftBorderThickness() and a
//							GetBottomRightBorderThickness() where before there was only
//							a GetTotalBorderThickness().  This also required a member
//							m_sInvertedBorder to remember whether the border was 
//							inverted.
//
//		09/24/96	JMI	Changed all BLU_MB?_* macros to RSP_MB?_* macros.
//
//		09/30/96	JMI	Added SetVisible() to handle visibility.  See summary
//							below.
//
//		10/01/96	JMI	Added GetVal().
//
//		10/01/96	JMI	~CGuiItem() no longer calls ms_listguiChildren.Remove()
//							since SetParent(NULL) does such.
//
//		10/01/96	JMI	Create() now calls Destroy().
//
//		10/22/96	JMI	Changed m_print to m_pprint which defaults to new
//							ms_print.  This should save much memory on CPRINTs.  They
//							are reasonably large and only need to be used multiply
//							if one wants to use multiple fonts or other settings for
//							their GUIs.
//
//		10/27/96 MJR	Fixed "unused variable" warnings.
//
//		10/31/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CImage			RImage
//							CGuiItem			RGuiItem
//							CList				RList
//							DRAWCALL			DrawCall
//							BACKCALL			BackCall
//							BTNUPCALL		BtnUpCall
//							CHot				RHot
//							LEFT				Left
//							CENTERED			Centered
//							CPRINT			RPRINT
//							CFNT				RFNT
//
//		11/01/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							RFNT				RFontOld		(soon will need to be RFont)
//							RPRINT			RPrint
//							Rect				RRect
//
//							Also, changed all members referenced in RImage to
//							m_ and all position/dimension members referenced in
//							RImage to type short usage.
//
//		11/11/96	JMI	Now m_szText is initialized to "" in CGuiItem::CGuiItem.
//
//		11/27/96	JMI	Added initialization of m_type to identify this type
//							of GUI item.
//
//		12/04/96	JMI	Now initializes RRects using new syntax.
//
//		12/19/96	JMI	Uses new m_justification (as m_sJustification) and 
//							upgraded to new RFont/RPrint.
//
//		12/22/96	JMI	Added support for transparent uncompressed buffers.
//							Now, if m_sTransparent is set and the m_im buffer
//							is a type that causes ImageIsCompressed() to return
//							0, rspBlitT will be used to blit the image
//							with transparent color m_u32TransparentColor.
//
//		12/28/96	JMI	Added CreateGuiItem() function that will allocate,
//							with new, the requested 'standard' RGuiItem or 
//							descendant.
//
//		12/29/96	JMI	Added m_lId member used to item this RGuiItem from
//							others.  Added GetItemFromId() to get an RGuiItem
//							via an ID.  Added 'Justified' to the Justification
//							enum.
//
//		12/29/96	JMI	Added DestroyGuiItem() function that will properly
//							and completely destroy a RGuiItem or descendant, if
//							the item is of a 'standard' type.
//							
//		12/30/96	JMI	Now SetActive() works the way SetVisible() always handled
//							activation.  On activation, it activates its RHot only
//							if m_sActive is TRUE.  On deactivation, it always
//							deactivates its RHot.
//							Now, if no clip rect specified to Draw(), the created 
//							clip rect is clipped to the image dimensions.
//
//		12/31/96	JMI	Added ms_pguiFocus static RGuiItem* and static functions
//							FocusNext(), FocusPrev(), and SetFocus() to standardize 
//							focus.  
//							Also, updated header comment regarding focus.
//							Note that ms_pguiFocus can be set by any other API; it
//							is not necessary to use FocusNext/Prev() and SetFocus(),
//							but for the sake of making as much work together as 
//							possible, it would be nice if we tried to stick to
//							these functions.
//
//		01/01/97	JMI	Added GetHot() which gets the area for the RHot and
//							now HotCall() will make your RHot as large as possible
//							on button down in order to increase the likelihood of 
//							catching the button up.  Also, compose now sets your
//							RHot to what GetHot() returns.
//							
//		01/02/97	JMI	Added DrawFocus() which draws a focus rectangle around
//							the border of the client area which is intended to show
//							which item has the focus.
//
//		01/03/97	JMI	Removed OffsetTopLevelPos().  Now everything is truly
//							parent-child, including RHots.  This greatly simplified
//							things with hotboxes.
//
//		01/04/97	JMI	Upgraded HotCall() to new CursorEvent().  This upgrade
//							is in response to RGuiItem now using relative RHots.
//							Now m_hot.m_sX/Y is parent item relative just like
//							m_sX/Y.  This should simplify a lot of stuff and even
//							fix some odd features like being able to click a GUI
//							item that exceeds the boundary of its parent.  This fix
//							will be essential for the not-yet-existent RListBox since
//							it will most likely scroll many children through a small
//							client area.
//							Now there are two regions associated with cursor events.
//							The first is the 'hot' area.  This is the area that m_hot
//							is set to include.  Child items can only receive cursor
//							events through this area.  The second is the 'event' area.
//							This is the area where the item really is actually con-
//							cerned with cursor events.  Example:  For a Dlg, the
//							entire window is the 'hot' area and the title bar is the
//							'event' area.
//
//		01/05/97	JMI	Now descends from RProps so users of this and descendant
//							classes can now add props.  Templated to item type U32
//							with key type U32.
//							Also, added virtual destructor to guarantee that all
//							the destructors of the _allocated_ type get called.
//
//		01/06/97	JMI	Now draws back to front and always clips to client area.
//							If a clipping rectangle is provided, Draw() intersects
//							it with the client area before recursing through child
//							items.  If no clipping rectangle is provided, Draw()
//							intersects the destination image's rectangle with the
//							client area before recursing through child items.
//
//		01/06/97	JMI	SetParent() now inserts new children at head of parent's
//							list.
//
//		01/13/97	JMI	Added RListBox to CreateGuiItem() and DestroyGuiItem().
//
//		01/14/97	JMI	Create() now calls Destroy() on failure instead of 
//							destroying m_im manually.
//							SetParent() now sets the activated status to that of the
//							new parent if there is one.  If there is none, the item
//							is deactivated.  I'm not sure if the latter is correct.
//							It seems as if, now that there's this parent/child
//							relationship between hot boxes, that items should just
//							use their m_hot for their activation status.
//							Also, with this parent/child relationship, there is no
//							need to set the m_hot priority based on the child depth.
//							It is now up to the particular control or use of such to
//							utilize the m_hot.SetPriority(), which has proved useful
//							already for the RListBox class.
//
//		01/14/97	JMI	Added Load and Save capabilities.  Still in early stages
//							but hopefully I did it correctly.  Seems to work fine.
//		
//		01/15/97	JMI	Now Load() and LoadInstantiate() call LoadChildren()
//							BEFORE Create().
//
//		01/16/97	JMI	Added IsClicked(), SetClicked(), and m_sClicked.
//
//		01/18/97	JMI	Added static DoFocus() to handle simple input focus.
//
//		01/20/97	JMI	Added GetItemFromPoint() to get a GUI from the tree that
//							contains the specified point.
//
//		01/21/97	JMI	Fixed comment in ReadMembers().  Added 'case' for new
//							version (version 1).  Move future cases to the top of
//							the switch (they were incorrectly on the bottom).
//
//		01/21/97	JMI	Added static array of strings describing types.
//
//		01/22/97	JMI	GetText() had the typical strcpy args backward error.
//
//		01/23/97	JMI	Add initialization of new member m_sFocusPos which
//							dictates where (relative to the client x, y) to draw
//							the focus.
//
//		02/05/97	JMI	Added CreateGuiItem() and DestroyGuiItem() cases for
//							new PushBtn (RPushBtn).
//							Also, hopefully fixed thickness problem in DrawBorder()
//							that occurred when thickness was greater than 1.
//							Same for GetTopLeft/BottomRightBorderThickness().
//
//		02/05/97	JMI	Added string in ms_apszTypes for RPushBtn.
//
//		02/05/97	JMI	Now SetParent() calls OnLoseChild() for parent losing
//							child and OnGainChild() for parent gaining child.
//							Also, ~RGuiItem() calls Destroy() after SetParent(NULL).
//
//		02/25/97	JMI	SaveChildren() now goes through the children in reverse
//							order so they, on load, get added back to their parent in
//							the order they were originally added to this parent.
//
//		03/13/97	JMI	Added SetText(long lId, char* pszFrmt, ...);
//							Added GetVal(long lId) and GetText(long lId, char*);
//
//		03/19/97	JMI	HotCall() now adapts sPosX,Y from hotbox coords instead
//							of hotbox's parent's coords.
//
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//		03/28/97	JMI	RSP_MB0_DOUBLECLICK is now treated the same as
//							RSP_MB0_PRESSED.
//
//		04/01/97	JMI	Changed short m_sCanBeFocused (TRUE, FALSE) to 
//							m_targetFocus (Self, Parent, Sibling).
//							Also, changed position of default case in ReadMembers().
//
//		04/02/97	JMI	TRACE message in GetText() was displaying a wrong value.
//
//		04/04/97	JMI	Upped GUI_FILE_VERSION to 3 for new RScrollBar members
//							regarding smooth scrollage.
//
//		04/10/97	JMI	Added components for background resource.  Also, upped
//							file version to save these components.
//							Added font cell height member (loads and saves it too).
//
//		04/10/97	JMI	Now if m_sBkdPlacement specifies Tile but not Center,
//							the whole background will be filled.
//
//		04/10/97	JMI	Added case MultiBtn for RMultiBtn and string description.
//
//		04/24/97	JMI	Added m_u32TextShadowColor.
//							Also, upped GUI_FILE_VERSION to 5 to read this member.
//
//		06/14/97	JMI	~RGuiItem() was checking this->IsProp(..) instead of
//							pgui->IsProp(..).  Fixed.
//
//		06/30/97 MJR	Moved SetVisible() into .cpp because the declaration of
//							a static inside of it was causing problems with mac
//							precompiled headers.
//
//		07/01/97	JMI	Moved a bunch of functions (some that cannot be
//							inlined (b/c they are virtual) ) into the guiItem.cpp.
//
//		07/01/97	JMI	Now CopyParms() now copies m_sFontCellHeight.
//
//		07/07/97	JMI	Added TextEffectsFlags, m_sTextEffects, m_sTextShadowOffsetX, 
//							and m_sTextShadowOffsetY.
//							Also, upped GUI_FILE_VERSION to 6 to save these new 
//							members.
//							Also, added SetTextEffects().
//
//		08/22/97	JMI	Now uses rspGeneralLock/Unlock() to make sure the 
//							destination buffer to Draw() is properly locked, if 
//							necessary.
//
//		09/12/97	JMI	Made ReadMembers() and WriteMembers() public.
//
//					JMI	Added GetCurrentFileVersion().
//
//		09/25/97	JMI	Upped GUI_FILE_VERSION to 7 so RMultiBtn can save its
//							current state which it always should've done.
//
//		10/06/99	JMI	Added m_fnInputEvent.
//
//////////////////////////////////////////////////////////////////////////////
//
// This is intended as a base class for all RSPiX GUI objects.  It does most
// of the basic functionality for a gui object from CHot callbacks to drawing
// borders and backgrounds.  Just about all of its funcitons can be overrided.
// If you want to create a totally unique GUI item, derive it from this.
// If you are making a button or other item that is similar to something 
// already implemented, you might want to derive from a slightly higher
// level such as RBtn, RTxt, RDlg, RScrollBar, etc.
//
// Visibility:
//	Visibility is handled in what may seem like a wierd way but, at this time,
// to me, it seems like a useful way.  I reserve the right to change my
// opinion.
// Effectively, calling SetVisible(TRUE) sets the visible status of this item
// and calls m_hot.SetActive(m_sActive) for this item and all children.
// The inverse is NOT true for SetVisible(FALSE).  SetVisible(FALSE) sets
// this item's m_sVisible to FALSE and calls m_hot.SetActive(FALSE) for this 
// and all child items.  The basic idea is that hiding a gui item causes this
// item and all chidren to not be displayed by setting m_sVisible, for this item
// only, to FALSE and deactivating mouse input for this and all children.
// Showing a gui item causes this item to be displayed by setting m_sVisible
// to TRUE.  All child items that have their m_sVisible set to TRUE will be
// displayed through the parent's Draw().  All child items that have their
// m_sActive set to TRUE will have mouse input activated.
// m_sVisible is synonymous to Windows' show state (through ShowWindow(SW_HIDE) and
// ShowWindow(SW_SHOW)).  m_sActive is synonymous to Windows' enabled state
// (through EnableWindow(FALSE | TRUE) ).
//
// Focus:
// There is very little support for focus.  It is considered a higher level
// idea.  What is provided is merely a static RGuiItem* that can be used to
// keep track of the item in focus and three functions, FocusNext(), 
// FocusPrev(), and SetFocus().  Note that FocusNext/Prev() only move the focus
// among children of one particular parent.  Also, since their is no list of 
// top-level guis in this model, the changing of focus among top-level guis 
// must be done without the use of FocusNext/Prev().  Currently, SetFocus()
// does barely anything, but it is better to use it than to set ms_pguiFocus
// manually b/c SetFocus() will probably eventually do something.
//
// Enhancements:
// To change the background of an RGuiItem, you can provide a callback in
// the member m_backcall instead of actually deriving a whole new class just
// for such purpose.  See m_backcall.
// Also, you can use a background resource name in m_szBkdResName to cause
// an image to be loaded into m_pimBkdRes which will be used in composing
// the item's background.  See GetRes() & ReleaseRes().
// Derive a class and override the DrawBorder to make different borders.
// Hopefully we can have fun with these.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers.
//////////////////////////////////////////////////////////////////////////////
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

//////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
//////////////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/guiItem.h"
	#include "GREEN/BLiT/BLIT.H"
	#include "ORANGE/IFF/iff.h"

	// Headers for CreateGuiItem.
	#include "ORANGE/GUI/txt.h"
	#include "ORANGE/GUI/btn.h"
	#include "ORANGE/GUI/edit.h"
	#include "ORANGE/GUI/scrollbar.h"
	#include "ORANGE/GUI/dlg.h"
	#include "ORANGE/GUI/ListBox.h"
	#include "ORANGE/GUI/PushBtn.h"
	#include "ORANGE/GUI/MultiBtn.h"
#else
	#include "BLIT.H"
	#include "GuiItem.h"

	// Headers for CreateGuiItem.
	#include "Txt.h"
	#include "Btn.h"
	#include "Edit.h"
	#include "ScrollBar.h"
	#include "Dlg.h"
	#include "ListBox.h"
	#include "iff.h"
	#include "PushBtn.h"
	#include "MultiBtn.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////
#define DEF_BORDER_COLOR				RSP_BLACK_INDEX
#define DEF_BORDER_SHADOW_COLOR		RSP_BLACK_INDEX
#define DEF_BORDER_HIGHLIGHT_COLOR	RSP_WHITE_INDEX
#define DEF_BORDER_EDGE_COLOR			RSP_WHITE_INDEX

#define DEF_BORDER_THICKNESS			1

#define DEF_TEXT_COLOR					RSP_WHITE_INDEX
#define DEF_BACK_COLOR					RSP_BLACK_INDEX
#define DEF_SHADOW_COLOR				RSP_BLACK_INDEX

#define DEF_FONT_CELL_HEIGHT			15

#define DEF_FOCUS_COLOR					RSP_WHITE_INDEX

#define BORDER_EFFECTS_THICKNESS		2

// Sets a value pointed to if ptr is not NULL.
#define SET(pval, val)					((pval != NULL) ? *pval = val : val)

// Finger print for GUI files.
#define GUI_FINGER_PRINT				MAKE_IFF_FCC('R', 'G', 'u', 'i')
#define GUI_FILE_VERSION				7

// If this prop is defined for a GUI item, it is 'delete'able.
#define DYNAMIC_PROP_KEY				3267021

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////
RPrint		RGuiItem::ms_print;			// This is the main RPrint that all
													// GUI items default to.
RGuiItem*	RGuiItem::ms_pguiFocus	= NULL;	// Higher level APIs can use this
															// as their current point of 
															// input focus.
char*			RGuiItem::ms_apszTypes[NumGuiTypes]	=	// Array of strings 
																	// indexed by type.
	{
	"GuiItem",
	"Txt",
	"Btn",
	"Edit",
	"ScrollBar",
	"Dlg",
	"ListBox",
	"PushBtn",
	"MultiBtn",
	};

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RGuiItem::RGuiItem()
	{
	m_sX	= 0;
	m_sY	= 0;

	m_hot.m_ulUser			= (ULONG)this;
	m_hot.m_iecUser		= HotCall;

	m_sEventAreaX			= 0;	// X coord of area in which we care
										// about cursor events.            
	m_sEventAreaY			= 0;	// Y coord of area in which we care
										// about cursor events.            
	m_sEventAreaW			= 0;	// Width of area in which we care  
										// about cursor events.            
	m_sEventAreaH			= 0;	// Height of area in which we care 
										// about cursor events.            
	
	m_sPressed				= FALSE;

	m_pguiParent			= NULL;

	m_drawcall				= NULL;
	m_backcall				= NULL;
	m_bcUser					= NULL;

	m_fnInputEvent			= NULL;

	m_ulUserInstance		= NULL;
	m_ulUserData			= 0;

	m_sBorderThickness	= DEF_BORDER_THICKNESS;

	m_u32BorderColor				= DEF_BORDER_COLOR;	
	m_u32BorderShadowColor		= DEF_BORDER_SHADOW_COLOR;
	m_u32BorderHighlightColor	= DEF_BORDER_HIGHLIGHT_COLOR;
	m_u32BorderEdgeColor			= DEF_BORDER_EDGE_COLOR;

	m_u32TextColor			= DEF_TEXT_COLOR;
	m_u32BackColor			= DEF_BACK_COLOR;
	m_u32TextShadowColor	= DEF_SHADOW_COLOR;

	m_sTextEffects			= 0;	// Flags for text effects.             
	                                                               
	m_sTextShadowOffsetX	= 1;	// Offset along X axis for text shadow.
	m_sTextShadowOffsetY	= 1;	// Offset along Y axis for text shadow.

	m_szText[0]				= '\0';

	m_sFontCellHeight		= DEF_FONT_CELL_HEIGHT;	// Cell height for text.

	// Default to left justified.
	m_justification		= RGuiItem::Left;

	m_sInvertedBorder		= FALSE;	// TRUE if border is inverted; FALSE
											// otherwise.                       

	m_type					= GuiItem;	// Indicates type of GUI item.

	m_sTransparent			= FALSE;	// TRUE, if this should be blt'ed via
											// a transparent blit call; FALSE,   
											// otherwise.  Note that this cannot 
											// override transparent Image formats
											// (e.g., FSPR8 will always be blt'ed
											// with transparency).               
	m_u32TransparentColor	= 0;	// Color used for transparency
											// when using transparent blit
											// call.                      

	m_lId						= 0;		// Default ID.

	m_targetFocus			= Self;	// Target when focus received.

	m_sShowFocus			= TRUE;	// TRUE if this item shows feedback,
											// usually via DrawFocus(), when it
											// has the focus.

	m_sFocusPos				= 0;		// Position at which DrawFocus() will
											// draw the focus rectangle relative
											// to the client area.  For example,
											// -1 would but it just outside the
											// client area.

	m_u32FocusColor		= DEF_FOCUS_COLOR;	// Color to draw focus with.

	m_sClicked				= FALSE;	// TRUE if this item is considered     
											// 'Clicked'.  For example, the default
											// implementation makes this TRUE if   
											// the cursor was pressed AND released 
											// within this item's hot area.        

	m_szBkdResName[0]		= '\0';	// Name of background res 
											// file to get into       
											// m_pimBkdRes.           
	m_sBkdResTransparent	= FALSE;	// TRUE, if m_pimBkdRes is to be
											// BLiT'ed transparently; FALSE,
											// otherwise.                   
	m_u32BkdResTransparentColor	= 0;	// Transparency color for 
													// m_pimBkdRes when       
													// BLiT'ed transparently. 
	m_sBkdResPlacement	= 0;		// Combination of |'ed             
											// Placement enum values.          
	m_pimBkdRes				= NULL;	// Background resource image.
	                                                        
	                                                        
	m_fnGetRes				= NULL;	// User callback to get background res
											// (m_pimBkdRes).                     
	m_fnReleaseRes			= NULL;	// User callback to release background
											// res (m_pimBkdRes).                 

	m_sVisible				= TRUE;	// TRUE if Draw() is to draw this item 	
											// and its children; FALSE, otherwise. 	
	m_sActive				= TRUE;	// TRUE if the CHot is to be activated
											// when visible.                      

	// Assign default (lowest GUI) priority.
	m_hot.SetPriority(0);

	// Set default RPrint.
	m_pprint					= &ms_print;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RGuiItem::~RGuiItem()
	{
	// Release all children.
	RGuiItem*	pgui	= m_listguiChildren.GetHead();
	while (pgui != NULL)
		{
		pgui->SetParent(NULL);

		// If the item has the dynamic prop set . . .
		if (pgui->IsDynamic() != FALSE)
			{
			// Destroy it.
			delete pgui;
			}

		pgui	= m_listguiChildren.GetNext();
		}

	// Release parent.
	SetParent(NULL);

	// If we were the item in focus . . .
	if (ms_pguiFocus == this)
		{
		// Kill focus.
		SetFocus(NULL);
		}

	// Destroy dynamic data.
	Destroy();

	// If we had a dynamic prop . . .
	if (IsProp(DYNAMIC_PROP_KEY) != FALSE)
		{
		// Remove the prop.
		RemoveProp(DYNAMIC_PROP_KEY);
		}
	}

////////////////////////////////////////////////////////////////////////
// Methods.
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Creates a displayable GUI item.
//
//////////////////////////////////////////////////////////////////////////////
short RGuiItem::Create(			// Returns 0 on success.
	short sX,						// X position relative to "parent" item.
	short sY,						// Y position relative to "parent" item.
	short sW,						// Width.
	short sH,						// Height.
	short sDepth)					// Color depth.
	{
	short	sRes	= 0;	// Assume success.

	Destroy();

	m_sX	= sX;
	m_sY	= sY;

	if (m_im.CreateImage(sW, sH, RImage::BMP8, 0, sDepth) == 0)
		{
		Compose();

		// Done.
		
		// If there's an error after calling CreateImage, perhaps
		// we should destroy the RImage data here.
		if (sRes != 0)
			{
			Destroy();
			}
		}
	else
		{
		TRACE("Create(): RImage::CreateImage() failed.\n");
		sRes = -1;
		}
	
	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Destroys dynamic display data.
// (virtual)
////////////////////////////////////////////////////////////////////////
void RGuiItem::Destroy(void)	// Returns nothing.
	{
	m_im.DestroyData();
	m_im.DestroyPalette();

	if (m_pimBkdRes != NULL)
		{
		ReleaseRes();

		ASSERT(m_pimBkdRes == NULL);
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Blit this item only into provided RImage.  Used by Draw().
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::Blit(		// Returns 0 on success.
	RImage* pimDst,			// Destination image.
	short sDstX,				// X position in destination.
	short sDstY,				// Y position in destination.
	short sSrcX /*= 0*/,		// X position in source.
	short sSrcY /*= 0*/,		// Y position in source.
	short sW /*= 0*/,			// Amount to draw.
	short sH /*= 0*/,			// Amount to draw.
	RRect* prc /*= NULL*/)	// Clip to.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pimDst != NULL);

	// If this is a sprite . . .
	if (ImageIsCompressed(m_im.m_type) != 0)
		{
		if (rspBlit(&m_im, pimDst, sDstX, sDstY, prc) == 0)
			{
			// Success.
			}
		else
			{
			TRACE("Blit(): rspBlit() failed.\n");
			sRes = -1;
			}
		}
	else
		{
		if (sW == 0)
			{
			sW	= m_im.m_sWidth;
			}

		if (sH == 0)
			{
			sH	= m_im.m_sHeight;
			}

		if (m_sTransparent == FALSE)
			{
			if (rspBlit(&m_im, pimDst, sSrcX, sSrcY, 
							sDstX, sDstY, 
							sW, sH, prc) == 0)		
				{
				// Success.
				}
#if 0		// Blit returns the same error for just about everything (including
			// source clipped out).
			else
				{
				TRACE("Blit(): rspBlit() failed.\n");
				sRes = -1;
				}
#endif
			}
		else
			{
			rspBlitT(
				m_u32TransparentColor,	// Transparent color.
				&m_im, pimDst,				// Src, Dst.
				sSrcX, sSrcY,
				sDstX, sDstY,
				sW, sH,
				prc,							// Dst clip.
				NULL);						// Src clip.
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Draw this item and all its subitems into the provided RImage.
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::Draw(		// Returns 0 on success.
	RImage* pimDst,			// Destination image.
	short sDstX	/*= 0*/,		// X position in destination.
	short sDstY	/*= 0*/,		// Y position in destination.
	short sSrcX /*= 0*/,		// X position in source.
	short sSrcY /*= 0*/,		// Y position in source.
	short sW /*= 0*/,			// Amount to draw.
	short sH /*= 0*/,			// Amount to draw.
	RRect* prc /*= NULL*/)	// Clip to.
	{
	short	sRes	= 0;	// Assume success.

	// If visible . . .
	if (m_sVisible != FALSE)
		{
		sDstX	+= m_sX;
		sDstY	+= m_sY;

		// Note that we lock this here and don't unlock it until after we
		// have processed our children.  Although and because Draw()
		// is recursive, this is the most efficient way to do this lock.
		// Since rspGeneralLock() (or, at least, the functions it calls) 
		// do not do anything if the buffer is already locked, there is
		// little efficiency lost in calling this multiple times but plenty
		// gained by not locking/unlocking and locking/unlocking again and
		// again (the exception being the flip page).
		rspGeneralLock(pimDst);

		if (Blit(pimDst, sDstX, sDstY, sSrcX, sSrcY, sW, sH, prc) == 0)
			{
			// An original clippage should be passed to all these children.
			// Set up default clip rect.
			RRect	rc;
			// Get client.
			GetClient(&rc.sX, &rc.sY, &rc.sW, &rc.sH);
			
			// Affect by draw position.
			rc.sX	+= sDstX;
			rc.sY	+= sDstY;
			
			// Clip to destination.
			RRect	rcDst;

			// If no rect . . .
			if (prc == NULL)
				{
				// Provide one using destination image.
				rcDst.sX	= 0;
				rcDst.sY	= 0;
				rcDst.sW	= pimDst->m_sWidth;
				rcDst.sH	= pimDst->m_sHeight;

				prc	= &rcDst;
				}

			// Clip default clipper to provided or destination.
			rc.ClipTo(prc);

			// Draw back to front!

			RGuiItem*	pgui	= m_listguiChildren.GetTail();
			
			while (sRes == 0 && pgui != NULL)
				{
				// Draw subitem and all its subitems.
				sRes	= pgui->Draw(pimDst, sDstX, sDstY, 0, 0, 0, 0, &rc);

				pgui	= m_listguiChildren.GetPrev();
				}

			// If this item has the focus . . .
			if (ms_pguiFocus == this)
				{
				// Draw focus feedback, if this item shows the focus.
				DrawFocus(pimDst, sDstX, sDstY, &rc);
				}
			}
		else
			{
			TRACE("Draw(): Blit() failed.\n");
			sRes = -1;
			}

		// Unlock the destination buffer.
		rspGeneralUnlock(pimDst);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Pass a message up to the highest level that we need to draw.
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::Redraw(	// Returns 0 on success.
	short	sSrcX /*= 0*/,	// X position to start drawing from.
	short	sSrcY /*= 0*/,	// Y position to start drawing from.
	short sW /*= 0*/,		// Amount to draw.
	short sH /*= 0*/)		// Amount to draw.
	{
	short	sRes	= 0;	// Assume success.

	// If there is a user callback . . .
	if (m_drawcall != NULL)
		{
		if (sW == 0)
			{
			sW = m_im.m_sWidth;
			}

		if (sH == 0)
			{
			sH = m_im.m_sHeight;
			}

		// If we have a parent . . .
		if (m_pguiParent != NULL)
			{
			sRes = m_pguiParent->Redraw(m_sX + sSrcX, m_sY + sSrcY, sW, sH);
			}
		else
			{
			// Highest level.  Parent of all parents.
			// Create a temporary image.
			RImage im;
			if (im.CreateImage(sW, sH, RImage::BMP8, 0, m_im.m_sDepth) == 0)
				{
				// Draw into image.
				Draw(&im, -m_sX, -m_sY, sSrcX, sSrcY, sW, sH);

				RRect	rc(m_sX + sSrcX, m_sY + sSrcY, sW, sH);
				
				// Pass on to user callback.
				(*m_drawcall)(this, &im, &rc);
		
				// Done with image.
				im.DestroyData();
				}
			else
				{
				TRACE("Redraw(): RImage::CreateImage() failed.\n");
				sRes	= -1;
				}
			}
		}
	else
		{
		sRes = 1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Ask user to erase area specified.
//
////////////////////////////////////////////////////////////////////////
void RGuiItem::Erase(	// Returns nothing.
	short sX /*= 0*/,		// X position to erase.
	short sY /*= 0*/,		// Y position to erase.
	short sW /*= 0*/,		// Width to erase.
	short sH /*= 0*/)		// Height to erase.
	{
	// If there is a callback . . .
	if (m_drawcall != NULL)
		{
		if (sW == 0)
			{
			sW	= m_im.m_sWidth;
			}

		if (sH == 0)
			{
			sH	= m_im.m_sHeight;
			}

		RRect	rc(m_sX + sX, m_sY + sY, sW, sH);

		// Ask user.
		(*m_drawcall)(this, NULL, &rc);
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Set the text that represents this item.
//
////////////////////////////////////////////////////////////////////////
void RGuiItem::SetText(	
	char* pszFrmt,	// sprintf formatted format string.
	...)				// Corresponding good stuff.
	{
	va_list val;
	va_start(val, pszFrmt);    
	  
	vsprintf(m_szText, pszFrmt, val);
	}

////////////////////////////////////////////////////////////////////////
// Set the text that represents the specified child item.
////////////////////////////////////////////////////////////////////////
short RGuiItem::SetText(	// Returns 0 if item found, non-zero otherwise.
	long	lId,					// Child item ID (can identify this item).
	char* pszFrmt,				// sprintf formatted format string.
	...)							// Corresponding good stuff.
	{
	short	sRes	= 0;	// Assume success.

	RGuiItem*	pgui	= GetItemFromId(lId);
	if (pgui != NULL)
		{
		va_list	val;
		va_start	(val, pszFrmt);

		vsprintf(pgui->m_szText, pszFrmt, val);
		}
	else
		{
		TRACE("SetText(): No such ID %ld.\n", lId);
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Sets up the current text effects on m_pprint to match this
// GUI's settings.
////////////////////////////////////////////////////////////////////////
void RGuiItem::SetTextEffects(void)	// Returns nothing.
	{
	// If shadow enabled . . .
	if (m_sTextEffects & Shadow)
		{
		m_pprint->SetEffectAbs(RPrint::SHADOW_X, m_sTextShadowOffsetX);
		m_pprint->SetEffectAbs(RPrint::SHADOW_Y, m_sTextShadowOffsetY);
		}
	else
		{
		m_pprint->SetEffectAbs(RPrint::SHADOW_X, 0);
		m_pprint->SetEffectAbs(RPrint::SHADOW_Y, 0);
		}

#if 0	// Not Yet Implemented -- time constraints.
	// If bold enabled . . .
	if (m_sTextEffects & Bold)
		{
		m_pprint->SetEffect(RPrint::BOLD, m_dTextBold);
		}
	else
		{
		m_pprint->SetEffect(RPrint::BOLD, 1.0);
		}

	// If italics enabled . . .
	if (m_sTextEffects & Italic)
		{
		m_pprint->SetEffect(RPrint::ITALIC, m_dTextItalic);
		}
	else
		{
		m_pprint->SetEffect(RPrint::ITALIC, 1.0);
		}
#endif
	}

////////////////////////////////////////////////////////////////////////
//
// Move this item to sX, sY.
//
////////////////////////////////////////////////////////////////////////
void RGuiItem::Move(	// Returns nothing.
	short sX,			// New x position.
	short sY)			// New y position.
	{
	// Erase old position.
	Erase();
	
	short	sDifX	= sX - m_sX;
	short	sDifY	= sY - m_sY;

	m_hot.m_sX	+= sDifX;
	m_hot.m_sY	+= sDifY;

	m_sX = sX; 
	m_sY = sY;

	// Not necessary if we are updated every iteration or frame.
	Redraw();
	}

////////////////////////////////////////////////////////////////////////
//
// Cursor event notification.
// Events in event area.
// (virtual).
//
////////////////////////////////////////////////////////////////////////
void RGuiItem::CursorEvent(	// Returns nothing.
	RInputEvent* pie)				// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.
	{
	switch (pie->sEvent)
		{
		case RSP_MB0_DOUBLECLICK:
		case RSP_MB0_PRESSED:
			// Remember we're pressed.
			m_sPressed	= TRUE;

			// Capture events.
			m_hot.SetCapture(TRUE);

			// Get focus.
			SetFocus();

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;

		case RSP_MB0_RELEASED:
			if (m_sPressed != FALSE)
				{
				// Stop capturing events.
				m_hot.SetCapture(FALSE);

				// If inside of item . . .
				if (	pie->sPosX >= m_sEventAreaX 
					&& pie->sPosX < m_sEventAreaX + m_sEventAreaW
					&&	pie->sPosY >= m_sEventAreaY 
					&& pie->sPosY < m_sEventAreaY + m_sEventAreaH)
					{
					// If there is a button up callback . . .
					if (m_bcUser != NULL)
						{
						(*m_bcUser)(this);
						}

					// Note clickage.
					SetClicked(TRUE);
					}

				m_sPressed	= FALSE;
				}

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Callback from CHot.
// Events in hot area.
//
////////////////////////////////////////////////////////////////////////
void RGuiItem::HotCall(	// Returns nothing.
	RInputEvent* pie)		// In:  Most recent user input event.             
								// Out: pie->sUsed = TRUE, if used.
	{
	// If this is an unused mouse event . . .
	if (pie->type == RInputEvent::Mouse && pie->sUsed == FALSE)
		{
		short	sHotPosX	= m_hot.m_sX;
		short	sHotPosY	= m_hot.m_sY;
		// Make relative to this GUI item (rather than hotbox).
		pie->sPosX	-= sHotPosX;
		pie->sPosY	-= sHotPosY;
		
		// If within event area or capturing . . .
		if (	(	pie->sPosX >= m_sEventAreaX && pie->sPosX < m_sEventAreaX + m_sEventAreaW
				&&	pie->sPosY >= m_sEventAreaY && pie->sPosY < m_sEventAreaY + m_sEventAreaH)
			||	m_hot.IsCapturing() != FALSE)
			{
			// Pass item relative event.
			CursorEvent(pie);
			}

		// If there is a button event callback . . .
		if (m_fnInputEvent)
			{
			// Process event via callback w/ relative positioning
			m_fnInputEvent(this, pie);
			}

		// Put position back.
		pie->sPosX	+= sHotPosX;
		pie->sPosY	+= sHotPosY;
		}
	}

////////////////////////////////////////////////////////////////////////
// Activate or deactivate mouse reaction for this gui item only.
// If the item has m_sActive set to TRUE, it will be activated.
////////////////////////////////////////////////////////////////////////
// virtual						// If you override this, call this base if possible.
void RGuiItem::SetActive(	// Returns nothing.
	short sActive)				// TRUE to make active, FALSE otherwise.
	{ 
	if (sActive != FALSE)
		{
		m_hot.SetActive(m_sActive);
		}
	else
		{
		m_hot.SetActive(FALSE);
		}
	}

////////////////////////////////////////////////////////////////////////
// Hide or show GUI item and child items.  Causes RHots to be
// deactivated for both this and child items.
// DOES NOT CHANGE THE VISIBLE STATUS OF CHILD ITEMS.
////////////////////////////////////////////////////////////////////////
//virtual							// If you override this, call this base if possible.
void RGuiItem::SetVisible(		// Returns nothing.
	short sVisible)				// TRUE to make visible, FALSE otherwise.
	{
	static short sSem	= 0;

	// Only parent gains/looses its visible status . . .
	if (++sSem == 1)
		{
		m_sVisible	= sVisible;
		}

	// Set activation to visibility combined with visibility attribute.
	SetActive(m_sVisible && sVisible);

	// Enum children.
	RGuiItem*	pguiChild	= m_listguiChildren.GetHead();
	while (pguiChild != NULL)
		{
		pguiChild->SetVisible(m_sVisible && sVisible);

		pguiChild	= m_listguiChildren.GetNext();
		}

	// Open the gate.
	sSem--;
	}

////////////////////////////////////////////////////////////////////////
//
// Change parent.  Removes from old parent's list and adds to new.
// New can be NULL (so can old).
//
////////////////////////////////////////////////////////////////////////
void RGuiItem::SetParent(RGuiItem* pguiParent)
	{
	short	sDifX	= 0;	// Difference in top level x positioning.
	short	sDifY	= 0;	// Difference in top level y positioning.

	// If there is an old . . .
	if (m_pguiParent != NULL)
		{
		// Let it know it's about to lose it's child.
		m_pguiParent->OnLoseChild(this);

		if (m_pguiParent->m_listguiChildren.Remove(this) == 0)
			{
			}
		else
			{
			TRACE("SetParent(): Unable to remove child from old parent.\n");
			}
		}

	// Set new parent.
	m_pguiParent	= pguiParent;

	// If there is a new . . .
	if (m_pguiParent != NULL)
		{
		// Let it know it's about to gain a child.
		m_pguiParent->OnGainChild(this);

		if (m_pguiParent->m_listguiChildren.InsertHead(this) == 0)
			{
			}
		else
			{
			TRACE("SetParent(): Unable to Add child to new parent.\n");
			}

		// Set this item's RHot as a child of its parent's.
		m_hot.SetParent(&(m_pguiParent->m_hot) );

		// Activate item based on new parent.
		// NOTE that this can screw up the ordering.
		SetActive(m_pguiParent->IsActivated());
		}
	else
		{
		// Set this item's as a global/top-level RHot.
		m_hot.SetParent(NULL);
		// Item becomes inactive.  Or should it keep its current activation?
		SetActive(FALSE);
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Draw border.
//
////////////////////////////////////////////////////////////////////////
void RGuiItem::DrawBorder(			// Returns nothing.
	RImage* pim	/*= NULL*/,			// Dest image, uses m_im if NULL.
	short sInvert	/*= FALSE*/)	// Inverts border if TRUE.
	{
	short	sVertShadowPos;
	short	sHorzShadowPos;
	short	sVertHighlightPos;
	short sHorzHighlightPos;
	short sVertEdgePos;
	short sHorzEdgePos;

	if (pim == NULL)
		{
		pim	= &m_im;
		}

	short sW	= pim->m_sWidth;
	short	sH	= pim->m_sHeight;

	m_sInvertedBorder	= sInvert;

	if (sInvert == FALSE)
		{
		sVertShadowPos			= sW - m_sBorderThickness;
		sHorzShadowPos			= sH - m_sBorderThickness;
		sVertHighlightPos		= 0;
		sHorzHighlightPos		= 0;
		sVertEdgePos			= sW - m_sBorderThickness * 2;
		sHorzEdgePos			= sH - m_sBorderThickness * 2;
		}
	else
		{
		sVertShadowPos			= 0;
		sHorzShadowPos			= 0;
		sVertHighlightPos		= sW - m_sBorderThickness;
		sHorzHighlightPos		= sH - m_sBorderThickness;
		sVertEdgePos			= m_sBorderThickness;
		sHorzEdgePos			= m_sBorderThickness;
		}

	// One pixel for each edge of border gets overwritten.
	rspRect(m_u32BorderHighlightColor, pim, 0, sHorzHighlightPos, sW, m_sBorderThickness);
	rspRect(m_u32BorderHighlightColor, pim, sVertHighlightPos, 0, m_sBorderThickness, sH);

	rspRect(m_u32BorderShadowColor, pim, 0, sHorzShadowPos, sW, m_sBorderThickness);
	rspRect(m_u32BorderShadowColor, pim, sVertShadowPos, 0, m_sBorderThickness, sH);

	rspRect(m_u32BorderEdgeColor, pim, m_sBorderThickness, sHorzEdgePos, sW - m_sBorderThickness * 2, m_sBorderThickness);
	rspRect(m_u32BorderEdgeColor, pim, sVertEdgePos, m_sBorderThickness, m_sBorderThickness, sH - m_sBorderThickness * 2);
	}

////////////////////////////////////////////////////////////////////////
//
// Draw background.  Calls user callback m_backcall if providd.
//
////////////////////////////////////////////////////////////////////////
void RGuiItem::DrawBackground(	// Returns nothing.
	RImage* pim	/*= NULL*/)			// Dest image, uses m_im if NULL.
	{
	if (pim == NULL)
		{
		pim	= &m_im;
		}

	RRect	rc;
	GetClient(&rc.sX, &rc.sY, &rc.sW, &rc.sH);
	if (m_backcall != NULL)
		{
		(*m_backcall)(this, pim, &rc);
		}
	else
		{
		// Draw "client".
		rspRect(	m_u32BackColor, pim,
					rc.sX, rc.sY, 
					rc.sW, rc.sH);
		
		DrawBackgroundRes(pim);
		}
	}

////////////////////////////////////////////////////////////////////////
// Draw background resource, if one is specified.
// Utilizes m_*BkdRes* parameters to get, place, and BLiT the resource.
// (virtual).
////////////////////////////////////////////////////////////////////////
void RGuiItem::DrawBackgroundRes(	// Returns nothing.
	RImage* pim /*= NULL*/)				// Dest image, uses m_im, if NULL.
	{
	if (pim == NULL)
		{
		pim	= &m_im;
		}

	// If resource not loaded . . .
	if (m_pimBkdRes == NULL)
		{
		// If there's a background res . . .
		if (m_szBkdResName[0] != '\0')
			{
			if (GetRes() == 0)
				{
				}
			else
				{
				TRACE("DrawBackgroundRes(): GetRes() failed.\n");
				}
			}
		}

	// If resource now available . . .
	if (m_pimBkdRes != NULL)
		{
		RRect	rc;
		GetClient(&rc.sX, &rc.sY, &rc.sW, &rc.sH);

		// Determine size to be drawn.
		short	sTotalW, sTotalH;
		if (m_sBkdResPlacement & Tile)
			{
			// Get amount to tile multiplied by the size of that dimension.
			sTotalW	= (rc.sW / m_pimBkdRes->m_sWidth) * m_pimBkdRes->m_sWidth;
			sTotalH	= (rc.sH / m_pimBkdRes->m_sHeight) * m_pimBkdRes->m_sHeight;
			// If not centered . . .
			if ((m_sBkdResPlacement & Center) == 0)
				{
				// Allow overblit.
				
				// If not a perfect fit . . .
				if ((rc.sW % m_pimBkdRes->m_sWidth) != 0)
					{
					sTotalW	+= m_pimBkdRes->m_sWidth;
					}

				if ((rc.sH % m_pimBkdRes->m_sHeight) != 0)
					{
					sTotalH	+= m_pimBkdRes->m_sHeight;
					}
				}
			}
		else
			{
			sTotalW	= m_pimBkdRes->m_sWidth;
			sTotalH	= m_pimBkdRes->m_sHeight;
			}

		// Determine place to be drawn.
		short	sBlitX, sBlitY;
		if (m_sBkdResPlacement & Center)
			{
			sBlitX	= rc.sX + rc.sW / 2 - sTotalW / 2;
			sBlitY	= rc.sY + rc.sH / 2 - sTotalH / 2;
			}
		else
			{
			sBlitX	= rc.sX;
			sBlitY	= rc.sY;
			}

		// Draw.
		short	sX, sY;
		short	sMaxX	= sBlitX + sTotalW;
		short	sMaxY	= sBlitY + sTotalH;
		for (sY = sBlitY; sY < sMaxY; sY += m_pimBkdRes->m_sHeight)
			{
			for (sX = sBlitX; sX < sMaxX; sX += m_pimBkdRes->m_sWidth)
				{
				if (m_sBkdResTransparent == FALSE)
					{
					rspBlit(
						m_pimBkdRes,		// Src.
						pim,					// Dst.
						sX,					// Dst.
						sY,					// Dst.
						&rc);					// Dst.
					}
				else
					{
					rspBlitT(
						m_u32BkdResTransparentColor,	// Src.
						m_pimBkdRes,						// Src.
						pim,									// Dst.
						0,										// Src.
						0,										// Src.
						sX,									// Dst.
						sY,									// Dst.
						m_pimBkdRes->m_sWidth,			// Both.
						m_pimBkdRes->m_sHeight,			// Both.
						&rc,									// Dst.
						NULL);								// Src.
					}
				}
			}
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Compose item.
//
////////////////////////////////////////////////////////////////////////
void RGuiItem::Compose(			// Returns nothing.
	RImage* pim /*= NULL*/)		// Dest image, uses m_im if NULL.
	{
	if (pim == NULL)
		{
		pim = &m_im;
		}

	// If there is a border . . .
	if (m_sBorderThickness > 0)
		{
		// Draw it.
		DrawBorder(pim, m_sInvertedBorder);
		}

	// Draw background.
	DrawBackground(pim);

	// Set hot area.
	SetHotArea();

	// Set event area.
	SetEventArea();
	}

////////////////////////////////////////////////////////////////////////
// Do one iteration of processing.  This is useful on items that need to
// poll frequently or do something every once in a while that is not
// triggered by an event.
////////////////////////////////////////////////////////////////////////
// virtual						// If you override this, call this base if possible.
void RGuiItem::Do(			// Returns nothing.
	RInputEvent* /*pie*/)	// In:  Most recent user input event.             
									// Out: pie->sUsed = TRUE, if used.
	{
	// Currently, this base does nothing.
	}

////////////////////////////////////////////////////////////////////////
// Set this item's event area.  This is the area where cursor events are
// interesting to the item.
////////////////////////////////////////////////////////////////////////
// virtual									// If you override this, call this base if possible.
void RGuiItem::SetEventArea(void)	// Returns nothing.
	{
	m_sEventAreaX	= 0;
	m_sEventAreaY	= 0;
	m_sEventAreaW	= m_im.m_sWidth;
	m_sEventAreaH	= m_im.m_sHeight;
	}

////////////////////////////////////////////////////////////////////////
// Set this item's hot area.  This should be the total dimensions of this
// item.
////////////////////////////////////////////////////////////////////////
// virtual								// If you override this, call this base if possible.
void RGuiItem::SetHotArea(void)	// Returns nothing.
	{
	// Get the hot area relative to this item.
	GetHotArea(&(m_hot.m_sX), &(m_hot.m_sY), &(m_hot.m_sW), &(m_hot.m_sH));
	
	// Convert to parent coordinates.
	m_hot.m_sX	+= m_sX;
	m_hot.m_sY	+= m_sY;
	}

////////////////////////////////////////////////////////////////////////
// Copy basic parameters regarding appearance and use from this item to 
// the specified one.
////////////////////////////////////////////////////////////////////////
// virtual						// If you override this, call this base if possible.
void RGuiItem::CopyParms(	// Returns nothing.
	RGuiItem* pguiDst)		// Destination for parameters from this item.
	{
	pguiDst->m_pprint							= m_pprint;

	pguiDst->m_u32BorderColor				= m_u32BorderColor;
	pguiDst->m_u32BorderShadowColor		= m_u32BorderShadowColor;
	pguiDst->m_u32BorderHighlightColor	= m_u32BorderHighlightColor;
	pguiDst->m_u32BorderEdgeColor			= m_u32BorderEdgeColor;

	pguiDst->m_u32TextColor					= m_u32TextColor;
	pguiDst->m_u32BackColor					= m_u32BackColor;

	pguiDst->m_sBorderThickness			= m_sBorderThickness;

	pguiDst->m_justification				= m_justification;

	pguiDst->m_sTransparent					= m_sTransparent;

	pguiDst->m_u32TransparentColor		= m_u32TransparentColor;

	pguiDst->m_u32FocusColor				= m_u32FocusColor;

	pguiDst->m_sFontCellHeight				= m_sFontCellHeight;
	}

////////////////////////////////////////////////////////////////////////
// Called by SetParent() when a GUI is losing a child item.
////////////////////////////////////////////////////////////////////////
// virtual							// If you override this, call this base, if possible.
void RGuiItem::OnLoseChild(	// Returns nothing.
	RGuiItem*	/*pguiChild*/)	// Child item we're about to lose.
	{
	}

////////////////////////////////////////////////////////////////////////
// Called by SetParent() when a GUI is gaining a child item.
////////////////////////////////////////////////////////////////////////
// virtual							// If you override this, call this base, if possible.
void RGuiItem::OnGainChild(	// Returns nothing.
	RGuiItem*	/*pguiChild*/)	// Child item we're about to gain.
	{
	}

////////////////////////////////////////////////////////////////////////
// Sets the 'clicked' status.
////////////////////////////////////////////////////////////////////////
void RGuiItem::SetClicked(	// Returns nothing.
	short sClicked)			// New 'clicked' status.
	{
	m_sClicked	= sClicked;
	}

////////////////////////////////////////////////////////////////////////
// Draws focus for item if m_sShowFocus is TRUE.
////////////////////////////////////////////////////////////////////////
// virtual						// If you override this, call this base if possible.
void RGuiItem::DrawFocus(	// Returns nothing.
	RImage* pimDst,			// Destination image.
	short sDstX	/*= 0*/,		// X offset in destination.
	short sDstY	/*= 0*/,		// Y offset in destination.
	RRect* prc /*= NULL*/)	// Clip to.
	{
	// If this item shows the focus . . .
	if (m_sShowFocus != FALSE)
		{
		short	sClientX, sClientY, sClientW, sClientH;
		GetClient(&sClientX, &sClientY, &sClientW, &sClientH);

		rspRect(
			MAX(m_sBorderThickness, (short)1),
			m_u32FocusColor,
			pimDst,
			sDstX + sClientX + m_sFocusPos,
			sDstY + sClientY + m_sFocusPos,
			sClientW - m_sFocusPos * 2,
			sClientH - m_sFocusPos * 2,
			prc);
		}
	}

////////////////////////////////////////////////////////////////////////
// Called by the static implementation of SetFocus() on the item gaining
// the focus.
// It is okay to call SetFocus() from this function.
////////////////////////////////////////////////////////////////////////
// virtual				// If you override this, call this base if possible.
void RGuiItem::OnGainFocus(void)
	{
	// Does this item want the focus . . .
	switch (m_targetFocus)
		{
		case Self:
			break;
		case Parent:
			SetFocus(GetParent() );
			break;
		case Sibling:
			FocusNext();
			break;
		case Child:
			SetFocus(m_listguiChildren.GetHead() );
			break;
		}
	}

////////////////////////////////////////////////////////////////////////
// Called by the static implementation of SetFocus() on the item losing
// the focus.
// It is okay to call SetFocus() from this function.
////////////////////////////////////////////////////////////////////////
// virtual				// If you override this, call this base if possible.
void RGuiItem::OnLoseFocus(void)
	{
	// Currently, this base does nothing.
	}

////////////////////////////////////////////////////////////////////////
// Sets the current focus GUI pointer to this GUI.
////////////////////////////////////////////////////////////////////////
RGuiItem* RGuiItem::SetFocus(void)	// Returns pointer to GUI losing the focus.
	{
	return SetFocus(this);
	}

////////////////////////////////////////////////////////////////////////
//
// Draw text in m_szText in m_u32TextColor with transparent
// background at sX, sY with sW and m_sJustification.
// Does nothing if m_szText is empty.
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::DrawText(	// Returns 0 on success.
	short sX,					// X position in image.
	short sY,					// Y position in image.
	short sW /*= 0*/,			// Width of text area.
	short	sH /*= 0*/,			// Height of test area.
	RImage* pim /*= NULL*/)	// Destination image.  NULL == use m_im.
	{
	short	sRes	= 0;	// Assume success.

	// Draw text.
	if (m_szText[0] != '\0')
		{
		if (pim == NULL)
			{
			// Use internal image.
			pim	= &m_im;
			}

		if (sW == 0)
			{
			GetClient(NULL, NULL, &sW, NULL);
			}

		if (sH == 0)
			{
			GetClient(NULL, NULL, NULL, &sH);
			}

		// Text support is currently 8 bit only.
		m_pprint->SetColor((short)m_u32TextColor, (short)0, m_u32TextShadowColor);

		// Set size.  Hopefully this won't do too much scaling for 
		// caching purposes but I'm not sure.
		m_pprint->SetFont(m_sFontCellHeight);

		SetJustification();
		SetTextEffects();
		m_pprint->SetDestination(pim);
		m_pprint->SetColumn(sX, sY, sW, sH);
		m_pprint->print(sX, sY, m_szText);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Gets the optional resource image named m_szBkdResName for this item 
// into m_pimBkdRes.
// Each call to this function should have a corresponding call to
// ReleaseRes().
// This function can result in a callback.
// (virtual).
////////////////////////////////////////////////////////////////////////
short RGuiItem::GetRes(void)
	{
	short	sRes	= 0;	// Assume success.

	// If there's a callback . . .
	if (m_fnGetRes != NULL)
		{
		// Use it instead.
		sRes	= m_fnGetRes(this);
		}
	else
		{
		// Release current resource, if any.
		// Default implementation allows 'Release' call even if no 'Get'
		// call has yet occurred.  This should not be relied upon when
		// using user Get/Releases.
		// There is, of course, the possibility that someone will define
		// a Release without a Get or vice-versa...I think that's a dumb
		// idea.
		ReleaseRes();

		m_pimBkdRes	= new RImage;
		if (m_pimBkdRes != NULL)
			{
			if (RFileEZLoad(
				m_pimBkdRes, 
				m_szBkdResName, 
				"rb", 
				RFile::LittleEndian) == 0)
				{
				// Success.
				}
			else
				{
				TRACE("GetRes(): Failed to open resource \"%s\".\n", m_szBkdResName);
				sRes	= -2;
				}

			// If any errors occurred after allocation . . .
			if (sRes != 0)
				{
				delete m_pimBkdRes;
				m_pimBkdRes	= NULL;
				}
			}
		else
			{
			TRACE("GetRes(): Failed to allocate new RImage for "
				"m_pimBkdRes.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Releases the optional resource image previously gotten by a call to
// GetRes().
// Each call to this function should correspond to a previous call to
// GetRes().  You should not call this function if the corresponding
// GetRes() failed.
// This function can result in a callback.
// (virtual).
////////////////////////////////////////////////////////////////////////
void RGuiItem::ReleaseRes(void)
	{
	// If there's a callback . . .
	if (m_fnReleaseRes != NULL)
		{
		// Use it instead.
		m_fnReleaseRes(this);
		}
	else
		{
		if (m_pimBkdRes != NULL)
			{
			delete m_pimBkdRes;
			m_pimBkdRes	= NULL;
			}
		}
	}

////////////////////////////////////////////////////////////////////////
// File Oriented.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// This will open the specified file with read access in an RFile and
// pass it to Load(RFile*).
// NOTE:  There is a static version of this function that will detect
// the type of the root item from the file and allocate an item of
// that type before loading.  Useful for loading generic *.GUI files.
// See LoadInstance(char*).
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::Load(	// Returns 0 on success.
	char* pszFileName)	// Name of file to load from.
	{
	short	sRes	= 0;	// Assume success.

	// Attempt to open specified file . . .
	RFile	file;
	if (file.Open(pszFileName, "rb", RFile::LittleEndian) == 0)
		{
		// Always use your member.
		sRes	= Load(&file);

		// Close the file.
		file.Close();
		}
	else
		{
		TRACE("Load(\"%s\"): Failed to open file.\n", pszFileName);
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// This will load the GUI tree (i.e., 'this' item and its children).
// If you override this function, you should call this base version
// to have it load the base class members.
// NOTE:  There is a static version of this function that will detect
// the type of the root item from the file and allocate an item of
// that type before loading.  Useful for loading generic *.GUI files.
// See LoadInstance(RFile*).
// (virtual).
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::Load(	// Returns 0 on success.
	RFile*	pfile)		// RFile open with read access to load from.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pfile->IsOpen() != FALSE);

	// Read header . . .
	U32	u32Version;
	Type	type;
	if (ReadHeader(pfile, &u32Version, &type) == 0)
		{
		ASSERT(type == m_type);

		// Read members . . .
		if (ReadMembers(pfile, u32Version) == 0)
			{
			// Determine current color depth.
			short	sDepth	= 8;	// Safety or something.
			rspGetVideoMode(&sDepth);

			if (LoadChildren(pfile) == 0)
				{
				// Create item . . .
				if (Create(
					m_sX,
					m_sY,
					m_im.m_sWidth,
					m_im.m_sHeight,
					sDepth) == 0)
					{
					// Success.
					}
				else
					{
					TRACE("Load(): Create() failed.\n");
					sRes	= -3;
					}
				}
			else
				{
				TRACE("Load(): LoadChildren() failed.\n");
				sRes	= -4;
				}
			}
		else
			{
			TRACE("Load(): ReadMembers() failed.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("Load(): ReadHeader() failed.\n");
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Instantiate a GUI tree from a file.  Allocates a GUI item of the
// proper (specified in the file) type and loads it using the specified
// file.
// (static).
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RGuiItem::LoadInstantiate(	// Returns newly allocated GUI item
													// on success or NULL on failure.
	char*	pszFileName)						// Name of file to instantiate from.
	{
	RGuiItem*	pgui	= NULL;	// Assume nothing.

	// Attempt to open specified file . . .
	RFile	file;
	if (file.Open(pszFileName, "rb", RFile::LittleEndian) == 0)
		{
		// Always use your member.
		pgui	= LoadInstantiate(&file);

		// Close the file.
		file.Close();
		}
	else
		{
		TRACE("LoadInstantiate(\"%s\"): Failed to open file.\n", pszFileName);
		}

	return pgui;
	}

////////////////////////////////////////////////////////////////////////
//
// Instantiate a GUI tree from a file.  Allocates a GUI item of the
// proper (specified in the file) type and loads it using the specified
// file.
// (static).
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RGuiItem::LoadInstantiate(	// Returns newly allocated GUI item
													// on success or NULL on failure.
	RFile*	pfile)							// Pointer to open GUI file.
	{
	short			sError	= 0;
	RGuiItem*	pgui		= NULL;	// Assume nothing.

	ASSERT(pfile->IsOpen() != FALSE);

	// Read header . . .
	U32	u32Version;
	Type	type;
	if (ReadHeader(pfile, &u32Version, &type) == 0)
		{
		// Allocate a GUI item of the appropriate type . . .
		pgui	= CreateGuiItem(type);
		if (pgui != NULL)
			{
			// Read members . . .
			if (pgui->ReadMembers(pfile, u32Version) == 0)
				{
				// Determine current color depth.
				short	sDepth	= 8;	// Safety or something.
				rspGetVideoMode(&sDepth);

				if (pgui->LoadChildren(pfile) == 0)
					{
					// Create item . . .
					if (pgui->Create(
						pgui->m_sX,
						pgui->m_sY,
						pgui->m_im.m_sWidth,
						pgui->m_im.m_sHeight,
						sDepth) == 0)
						{
						// Success.
						}
					else
						{
						TRACE("LoadInstantiate(): Create() failed.\n");
						sError	= 4;
						}
					}
				else
					{
					TRACE("LoadInstantiate(): LoadChildren() failed.\n");
					sError	= 5;
					}
				}
			else
				{
				TRACE("LoadInstantiate(): ReadMembers() failed.\n");
				sError	= 3;
				}

			// If any errors occurred after allocating GUI . . .
			if (sError != 0)
				{
				delete pgui;
				pgui	= NULL;
				}
			}
		else
			{
			TRACE("LoadInstantiate(): CreateGuiItem() failed.\n");
			sError	= 2;
			}
		}
	else
		{
		TRACE("LoadInstantiate(): ReadHeader() failed.\n");
		sError	= 1;
		}

	return pgui;
	}

////////////////////////////////////////////////////////////////////////
//
// Load item's children from the specified file.
// (virtual).
// (protected).
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::LoadChildren(	// Returns 0 on success.
	RFile*	pfile)				// File to load from.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pfile->IsOpen() != FALSE);

	short	sNum;
	// Read number of children.
	pfile->Read(&sNum);

	// Instantiate children.
	RGuiItem* pgui;
	short	sCurChild;
	for (	sCurChild	= 0; 
			sCurChild < sNum && sRes == 0 && pfile->Error() == FALSE; 
			sCurChild++)
		{
		pgui	= LoadInstantiate(pfile);
		if (pgui != NULL)
			{
			pgui->SetParent(this);
			}
		else
			{
			TRACE("LoadChildren(): LoadInstantiate() failed.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Read header for this GUI item.
// (protected).
// (static).
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::ReadHeader(	// Returns 0 on success.
	RFile*	pfile,				// In:  File to read from.
	U32*	pu32Version,			// Out: File format version.
	Type*	ptype)					// Out: Type of GUI item stored.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pfile->IsOpen() != FALSE);

	// Read fingerprint.
	U32	u32FingerPrint;
	if (pfile->Read(&u32FingerPrint) == 1)
		{
		if (u32FingerPrint == GUI_FINGER_PRINT)
			{
			// Read file format version.
			if (pfile->Read(pu32Version) == 1)
				{
				// Read type.
				U32	u32Type;
				if (pfile->Read(&u32Type) == 1)
					{
					// Store type for caller.
					*ptype	= (Type)u32Type;
					}
				else
					{
					TRACE("ReadHeader(): Error reading GUI item type from file.\n");
					sRes	= -4;
					}
				}
			else
				{
				TRACE("ReadHeader(): Error reading file format version number.\n");
				sRes	= -3;
				}
			}
		else
			{
			TRACE("ReadHeader(): Invalid finger print.  Not a GUI file.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("ReadHeader(): Error reading finger print.\n");
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Read item's members from file.
// (virtual).
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::ReadMembers(	// Returns 0 on success.
	RFile*	pfile,				// File to read from.
	U32		u32Version)			// File format version to use.
	{
	short	sRes	= 0;	// Assume success.

	// Read members.  Type must always be first becuase 
	// LoadInstantiate() utilizes this fact to determine the type of item
	// that should be instantiated.
	U32	u32Temp;
	
	switch (u32Version)
		{
		default:
		// Insert additional version numbers here!
		case 6:	// Version 6 stuff.
			pfile->Read(&m_sTextEffects);
			pfile->Read(&m_sTextShadowOffsetX);
			pfile->Read(&m_sTextShadowOffsetY);

		case 5:	// Version 5 stuff.
			pfile->Read(&m_u32TextShadowColor);

		case 4:	// Version 4 stuff.
			// New members regarding background resources.
			pfile->Read(m_szBkdResName);
			pfile->Read(&m_sBkdResTransparent);
			pfile->Read(&m_u32BkdResTransparentColor);
			pfile->Read(&m_sBkdResPlacement);
			// New member regarding font size (height).
			pfile->Read(&m_sFontCellHeight);

		case 3:	// Version 3 stuff.
			// No new stuff here.  RScrollBar has some new members to load.
		case 2:	// Version 2 stuff.
			// No new stuff here.  RListBox now saves its m_typeEncapsulator.

		case 1:	// Version 1 stuff.
			// No new stuff here.  Derived types RListBox, RScrollBar, and
			// REdit load their members in version 1.

		case 0:	// Version 0 stuff.
			// Base class members.

			pfile->Read(&m_sX);
			pfile->Read(&m_sY);
			pfile->Read(&m_im.m_sWidth);
			pfile->Read(&m_im.m_sHeight);
			pfile->Read(m_szText);
			pfile->Read(&m_u32BorderColor);
			pfile->Read(&m_u32BorderShadowColor);
			pfile->Read(&m_u32BorderHighlightColor);
			pfile->Read(&m_u32BorderEdgeColor);
			pfile->Read(&m_u32TextColor);
			pfile->Read(&m_u32BackColor);
			pfile->Read(&m_sBorderThickness);

			pfile->Read(&u32Temp);
			m_justification	= (Justification)u32Temp;

			pfile->Read(&m_sInvertedBorder);
			pfile->Read(&m_sTransparent);
			pfile->Read(&m_u32TransparentColor);
			pfile->Read(&m_lId);
			
			// Used to load m_sCanBeFocused (TRUE, FALSE); now load
			// m_targetFocus.
			short	sFocusTarget	= Parent;
			pfile->Read(&sFocusTarget);
			m_targetFocus	=	(Target)sFocusTarget;

			pfile->Read(&m_sShowFocus);
			pfile->Read(&m_u32FocusColor);
			pfile->Read(&m_sVisible);
			pfile->Read(&m_sActive);

			// If successful . . .
			if (pfile->Error() == FALSE)
				{
				// Success.
				}
			else
				{
				TRACE("ReadMembers(): Error reading RGuiItem members.\n");
				sRes	= -1;
				}
			break;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// This will open the specified file with write access in an RFile and
// pass it to Save(RFile*).
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::Save(	// Returns 0 on success.
	char* pszFileName)	// Name of file to save to.
	{
	short	sRes	= 0;	// Assume success.

	// Attempt to open specified file . . .
	RFile	file;
	if (file.Open(pszFileName, "wb", RFile::LittleEndian) == 0)
		{
		// Always use your member.
		sRes	= Save(&file);

		// Close the file.
		file.Close();
		}
	else
		{
		TRACE("Save(\"%s\"): Failed to open file.\n", pszFileName);
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// This will save the GUI tree (i.e., 'this' item and its children).
// If you override this function, you should call this base version
// to have it save the base class members.
// (virtual).
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::Save(	// Returns 0 on success.
	RFile*	pfile)		// RFile open with write access to save to.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pfile->IsOpen() != FALSE);

	// Write fingerprint.
	pfile->Write((U32)GUI_FINGER_PRINT);
	// Write file format version.
	pfile->Write((U32)GUI_FILE_VERSION);

	// Write member variables for RGuiItem . . .
	if (WriteMembers(pfile) == 0)
		{
		// Write all children.
		sRes	= SaveChildren(pfile);
		}
	else
		{
		TRACE("Save(): Error writing RGuiItem members.\n");
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Save item's children to the specified file.
// (virtual).
// (protected).
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::SaveChildren(	// Returns 0 on success.
	RFile*	pfile)				// File to save to.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pfile->IsOpen() != FALSE);

	// Determine number of child items.
	short	sNum	= 0;
	RGuiItem*	pgui = m_listguiChildren.GetHead();
	while (pgui != NULL)
		{
		sNum++;

		pgui	= m_listguiChildren.GetNext();
		}

	// Write number of children.
	pfile->Write(sNum);

	// Save children.  Note that we go through the children in reverse
	// order so they, on load, get added back to their parent in the
	// order they were originally added to this parent.
	pgui	= m_listguiChildren.GetTail();
	while (pgui != NULL && sRes == 0 && pfile->Error() == FALSE)
		{
		sRes	= pgui->Save(pfile);

		pgui	= m_listguiChildren.GetPrev();
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Write item's members to file.
// (virtual).
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::WriteMembers(	// Returns 0 on success.
	RFile*	pfile)				// File to write to.
	{
	short	sRes	= 0;	// Assume success.

	// Write members.  Type must always be first becuase 
	// LoadInstantiate() utilizes this fact to determine the type of item
	// that should be instantiated.
	pfile->Write((U32)m_type);

	// Insert new members here.
	pfile->Write(m_sTextEffects);
	pfile->Write(m_sTextShadowOffsetX);
	pfile->Write(m_sTextShadowOffsetY);
	pfile->Write(m_u32TextShadowColor);
	pfile->Write(m_szBkdResName);
	pfile->Write(m_sBkdResTransparent);
	pfile->Write(m_u32BkdResTransparentColor);
	pfile->Write(m_sBkdResPlacement);
	pfile->Write(m_sFontCellHeight);
	pfile->Write(m_sX);
	pfile->Write(m_sY);
	pfile->Write(m_im.m_sWidth);
	pfile->Write(m_im.m_sHeight);
	pfile->Write(m_szText);
	pfile->Write(m_u32BorderColor);
	pfile->Write(m_u32BorderShadowColor);
	pfile->Write(m_u32BorderHighlightColor);
	pfile->Write(m_u32BorderEdgeColor);
	pfile->Write(m_u32TextColor);
	pfile->Write(m_u32BackColor);
	pfile->Write(m_sBorderThickness);
	pfile->Write((U32)m_justification);
	pfile->Write(m_sInvertedBorder);
	pfile->Write(m_sTransparent);
	pfile->Write(m_u32TransparentColor);
	pfile->Write(m_lId);
	pfile->Write((short)m_targetFocus);
	pfile->Write(m_sShowFocus);
	pfile->Write(m_u32FocusColor);
	pfile->Write(m_sVisible);
	pfile->Write(m_sActive);

	// If successful . . .
	if (pfile->Error() == FALSE)
		{
		// Success.
		}
	else
		{
		TRACE("WriteMembers(): Error writing RGuiItem members.\n");
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Querries.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Determine if the position is inside the item.
//
////////////////////////////////////////////////////////////////////////
inline short IsInside(	// Returns TRUE, if inside; FALSE otherwise.
	RGuiItem*	pgui,		// GUI in question.
	short			sPosX,	// X coord of position in question.
	short			sPosY,	// Y coord of position in question.
	short			sActive,	// If TRUE, only searches active items.
								// If FALSE, searches all items.
	short		sEventArea)	// If TRUE, only checks items' event areas.
								// If FALSE, checks items' entire hot regions.
	{
	short	sRes	= TRUE;	// Assume inside.

	// If activated is required . . .
	if (sActive != FALSE)
		{
		// Get activation status.
		sRes	= pgui->IsActivated();
		}

	// If okay so far . . .
	if (sRes != FALSE)
		{
		// If using event area . . .
		if (sEventArea != FALSE)
			{
			if (	sPosX < pgui->m_sEventAreaX
				||	sPosX >= pgui->m_sEventAreaX + pgui->m_sEventAreaW
				|| sPosY < pgui->m_sEventAreaY
				|| sPosY >= pgui->m_sEventAreaY + pgui->m_sEventAreaH)
				{
				sRes	= FALSE;
				}
			}
		else	// Entire hot area.
			{
			if (	sPosX < pgui->m_hot.m_sX
				||	sPosX >= pgui->m_hot.m_sX + pgui->m_hot.m_sW
				|| sPosY < pgui->m_hot.m_sY
				|| sPosY >= pgui->m_hot.m_sY + pgui->m_hot.m_sH)
				{
				sRes	= FALSE;
				}
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Get the item at the specified point that has the
// specified parameters.
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RGuiItem::GetItemFromPoint(	// Returns item ptr, if item found;
													// NULL on failure.
	short	sPosX,								// X position.
	short	sPosY,								// Y position.
	short	sActive /*= TRUE*/,				// If TRUE, only searches active items.
													// If FALSE, searches all items.
	short sEventArea /*= TRUE*/)			// If TRUE, only checks items' event areas.
													// If FALSE, checks items' entire hot regions.
	{
	RGuiItem*	pguiRes	= NULL;	// Assume nothing.

	// If inside this item . . .
	if (IsInside(this, sPosX, sPosY, sActive, sEventArea) != FALSE)
		{
		RGuiItem*	pguiCur;
		pguiRes	= this;	// Start with this.

		do
			{
			// Adapt coords to current item's system.
			sPosX	-= pguiRes->m_sX;
			sPosY	-= pguiRes->m_sY;

			// Check children.
			pguiCur	= pguiRes->m_listguiChildren.GetHead();
			while (pguiCur != NULL)
				{
				if (IsInside(pguiCur, sPosX, sPosY, sActive, sEventArea) != FALSE)
					{
					// Found one.
					pguiRes	= pguiCur;
					break;
					}

				pguiCur	= pguiRes->m_listguiChildren.GetNext();
				}

			// until we find no child item containing point.
			} while (pguiCur != NULL);
		}

	return pguiRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Returns TRUE if this item was allocated with CreateGuiItem().
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::IsDynamic(void)	// Returns TRUE if this item was allocated with
											// CreateGuiItem().
	{
	return IsProp(DYNAMIC_PROP_KEY);
	}

////////////////////////////////////////////////////////////////////////
//
// Get the text that represents this item.
//
////////////////////////////////////////////////////////////////////////
short RGuiItem::GetText(	// Returns 0 on success.
	char* pszText,				// Location to copy text to.
	short sMax)					// Total memory pointed to by pszText.
	{
	short	sRes	= 0;	// Assume success.

	if ((short)strlen(m_szText) < sMax)
		{
		strcpy(pszText, m_szText);
		}
	else
		{
		TRACE("GetText(): Not enough room to copy text (provided: %d, "
				"needed: %d).\n", sMax, strlen(m_szText) + 1);
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the text that represents the specified item.
//
//////////////////////////////////////////////////////////////////////////////
short RGuiItem::GetText(	// Returns 0 on success.
	long	lId,					// In:  Child item ID (can identify this item). 
	char* pszText,				// Out: Location to copy text to.
	short sMax)					// In:  Total memory pointed to by pszText.
	{
	short	sRes	= 0;	// Assume success.

	RGuiItem*	pgui	= GetItemFromId(lId);
	if (pgui != NULL)
		{
		sRes	= pgui->GetText(pszText, sMax);
		}
	else
		{
		TRACE("GetText(): No such ID %ld.\n", lId);
		sRes	= -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the number represented by the text in this item.
//
//////////////////////////////////////////////////////////////////////////////
long RGuiItem::GetVal(void)		// Returns value.
	{
	return atol(m_szText);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the number represented by the text in the specified item.
//
//////////////////////////////////////////////////////////////////////////////
long RGuiItem::GetVal(	// Returns value.
	long	lId)				// In:  Child item ID (can identify this item). 
	{
	long lRes	= 0;	// Assume value.  Ayuh.

	RGuiItem*	pgui	= GetItemFromId(lId);
	if (pgui != NULL)
		{
		lRes	= pgui->GetVal();
		}
	else
		{
		TRACE("GetVal(): No such ID %ld.\n", lId);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Gets the thickness of the top/left border (including border edge effect).
//
//////////////////////////////////////////////////////////////////////////////
short RGuiItem::GetTopLeftBorderThickness(void)	// Returns border thickness 
																// including edge effect.                      
	{
	if (m_sBorderThickness == 0)
		return 0;
	else
		return m_sBorderThickness + ((m_sInvertedBorder == FALSE) ? 0 : m_sBorderThickness);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Gets the thickness of the bottom/right border (including border edge effect).
//
//////////////////////////////////////////////////////////////////////////////
short RGuiItem::GetBottomRightBorderThickness(void)	// Returns border thickness 
																		// including edge effect.                      
	{
	if (m_sBorderThickness == 0)
		return 0;
	else
		return m_sBorderThickness + ((m_sInvertedBorder == FALSE) ? m_sBorderThickness : 0);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the "client" area (i.e., non-border/title area) relative to this
// item.
// (virtual).
//
//////////////////////////////////////////////////////////////////////////////
void RGuiItem::GetClient(	// Returns nothing.
	short* psX,					// Out: X position unless NULL.
	short* psY,					// Out: Y position unless NULL.
	short* psW,					// Out: Width unless NULL.
	short* psH)					// Out: Height unless NULL.
	{
	if (m_sBorderThickness == 0)
		{
		SET(psX, 0);
		SET(psY, 0);
		SET(psW, m_im.m_sWidth);
		SET(psH, m_im.m_sHeight);
		}
	else
		{
		short	sLeftTop			= GetTopLeftBorderThickness();
		short sRightBottom	= GetBottomRightBorderThickness();
		SET(psX, sLeftTop);
		SET(psY, sLeftTop);
		SET(psW, m_im.m_sWidth - sLeftTop - sRightBottom);
		SET(psH, m_im.m_sHeight - sLeftTop - sRightBottom);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the "hot" area (i.e., clickable area) relative to this item.
// Only children within this area will be able to get mouse events.
// (virtual).
//
//////////////////////////////////////////////////////////////////////////////
void RGuiItem::GetHotArea(	// Returns nothing.
	short* psX,					// Out: X position unless NULL.
	short* psY,					// Out: Y position unless NULL.
	short* psW,					// Out: Width unless NULL.
	short* psH)					// Out: Height unless NULL.
	{
	// Base implementation uses entire area.
	SET(psX, 0);
	SET(psY, 0);
	SET(psW, m_im.m_sWidth);
	SET(psH, m_im.m_sHeight);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Change the position specified to a top-level coord.
//
//////////////////////////////////////////////////////////////////////////////
void RGuiItem::ChildPosToTop(	// Returns nothing.
	short* psX,						// In: Child pos, Out: Top level pos.
	short* psY)						// In: Child pos, Out: Top level pos.
	{
	// Offset current pos.
	*psX	+= m_sX;
	*psY	+= m_sY;

	// If not yet the top level . . .
	if (m_pguiParent != NULL)
		{
		// Pass it on.
		m_pguiParent->ChildPosToTop(psX, psY);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Change the position specified to a child coord.
//
//////////////////////////////////////////////////////////////////////////////
void RGuiItem::TopPosToChild(	// Returns nothing.
	short* psX,						// In: Top level pos, Out: Child pos.
	short* psY)						// In: Top level pos, Out: Child pos.
	{
	// Offset current pos.
	*psX	-= m_sX;
	*psY	-= m_sY;

	// If not yet the top level . . .
	if (m_pguiParent != NULL)
		{
		// Pass it on.
		m_pguiParent->TopPosToChild(psX, psY);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the current file version (that is, the file version a file
// written from this GUI right now would be).
// (static).
//
//////////////////////////////////////////////////////////////////////////////
U32 RGuiItem::GetCurrentFileVersion(void)
	{
	return GUI_FILE_VERSION;
	}

//////////////////////////////////////////////////////////////////////////////
// Statics.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Sets the current focus GUI pointer to the supplied GUI.
// NOTE:  Eventually, this function may do more, so it is better to call
// it than simply setting ms_pguiFocus to your gui.
//////////////////////////////////////////////////////////////////////////////
// static 
RGuiItem* RGuiItem::SetFocus(		// Returns pointer to GUI losing the focus.
	RGuiItem*	pguiNewFocus)		// New item to focus or NULL for none.
	{
	// Store loser of focus.
	RGuiItem*	pguiLoseFocus	= ms_pguiFocus;

	// If this item does not already have the focus . . .
	if (pguiNewFocus != ms_pguiFocus)
		{
		// Set new focus.
		ms_pguiFocus	= pguiNewFocus;

		// If there is a loser . . .
		if (pguiLoseFocus != NULL)
			{
			// Let the loser know.
			pguiLoseFocus->OnLoseFocus();
			}
		
		// If the focus has not already changed and is an item . . .
		if (ms_pguiFocus == pguiNewFocus && pguiNewFocus != NULL)
			{
			// Let the gainer know.
			pguiNewFocus->OnGainFocus();
			}
		}

	// Return loser of focus.
	return pguiLoseFocus;
	}

//////////////////////////////////////////////////////////////////////////////
// Moves the focus to the next GUI at the same level as the current
// ms_pguiFocus.  Does not affect current focus for top-level guis.
//////////////////////////////////////////////////////////////////////////////
// static 
RGuiItem* RGuiItem::FocusNext(void)		// Returns new item with focus or NULL,
													// if none.
	{
	// If there is a current . . .
	if (ms_pguiFocus != NULL)
		{
		// If it has a parent . . .
		RGuiItem*	pguiParent	= ms_pguiFocus->GetParent();
		if (pguiParent != NULL)
			{
			// Get next . . .
			RGuiItem* pguiFocus = pguiParent->m_listguiChildren.GetLogicalNext(ms_pguiFocus);
			// Remember where we started so we can detect complete circle.
			RGuiItem* pguiStart = pguiFocus;
			
			while (pguiFocus != NULL)
				{
				// If visible, activated, and doesn't focus siblings . . .
				if (	pguiFocus->m_sVisible != FALSE 
					&&	pguiFocus->IsActivated() != FALSE
					&&	pguiFocus->m_targetFocus != Sibling)
					{
					break;
					}

				// Try next.
				pguiFocus	= pguiParent->m_listguiChildren.GetLogicalNext();

				// If we wrapped . . .
				if (pguiFocus == pguiStart)
					{
					pguiFocus	= NULL;
					}
				}

			// Set focus to the new item.
			SetFocus(pguiFocus);
			}
		}

	return ms_pguiFocus;
	}

//////////////////////////////////////////////////////////////////////////////
// Moves the focus to the previous GUI at the same level as the current
// ms_pguiFocus.  Does not affect current focus for top-level guis.
//////////////////////////////////////////////////////////////////////////////
// static 
RGuiItem* RGuiItem::FocusPrev(void)		// Returns new item with focus or NULL,
													// if none.
	{
	// If there is a current . . .
	if (ms_pguiFocus != NULL)
		{
		// If it has a parent . . .
		RGuiItem*	pguiParent	= ms_pguiFocus->GetParent();
		if (pguiParent != NULL)
			{
			// Get previous . . .
			RGuiItem* pguiFocus = pguiParent->m_listguiChildren.GetLogicalPrev(ms_pguiFocus);
			// Remember where we started so we can detect complete circle.
			RGuiItem* pguiStart = pguiFocus;
			
			while (pguiFocus != NULL)
				{
				// If visible, activated, and doesn't focus siblings . . .
				if (	pguiFocus->m_sVisible != FALSE 
					&&	pguiFocus->IsActivated() != FALSE
					&&	pguiFocus->m_targetFocus != Sibling)
					{
					break;
					}

				// Try prev.
				pguiFocus	= pguiParent->m_listguiChildren.GetLogicalPrev();

				// If we wrapped . . .
				if (pguiFocus == pguiStart)
					{
					pguiFocus	= NULL;
					}
				}

			// Set focus to the new item.
			SetFocus(pguiFocus);
			}
		}

	return ms_pguiFocus;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sends the event to the GUI focused via Do(), and, if event is not 
// used, checks for  keys that might change the focus.
// (static).
//
//////////////////////////////////////////////////////////////////////////////
void RGuiItem::DoFocus(	// Returns nothing.
	RInputEvent* pie)		// In:  Most recent user input event.             
								// Out: pie->sUsed = TRUE, if used.
	{
	// If there is an item with the focus . . .
	if (ms_pguiFocus != NULL)
		{
		// Pass event on.
		ms_pguiFocus->Do(pie);
		}

	// If unused key event . . .
	if (pie->sUsed == FALSE && pie->type == RInputEvent::Key)
		{
		// Assume used.  Easier.
		pie->sUsed	= TRUE;

		switch (pie->lKey)
			{
			// Tab == Next item at current level.
			case '\t':							
				FocusNext();
				break;

			// Shift-Tab == Prev item at current level.
			case (RSP_GKF_SHIFT | '\t'):
				FocusPrev();
				break;

			// Control-Tab == Child.
			case (RSP_GKF_CONTROL | '\t'):
				// If there's currently an item in focus . . .
				if (ms_pguiFocus != NULL)
					{
					// Get first child in list of children.
					SetFocus(ms_pguiFocus->m_listguiChildren.GetHead() );
					}
				break;

			// Control-Shift-Tab == Parent.
			case (RSP_GKF_CONTROL | RSP_GKF_SHIFT | '\t'):
				// If there's currently an item in focus . . .
				if (ms_pguiFocus != NULL)
					{
					// Get parent.
					SetFocus(ms_pguiFocus->GetParent() );
					}
				break;

			default:	// Unused.
				// Well, better set it back to unused.
				pie->sUsed	= FALSE;
				break;
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Creates the specified object and returns a pointer to it.
// The object is created with new, so use delete to destroy the object.
// (static).
//
//////////////////////////////////////////////////////////////////////////////
RGuiItem* RGuiItem::CreateGuiItem(	// Returns the allocated type on success.
												// Returns NULL on error.
	Type	type)								// Type of GuiItem to allocate.  Must be one
												// of the enums that is a member of 
												// RGuiItem::Type.
	{
	RGuiItem*	pgui	= NULL;	// Assume failure.
	switch (type)
		{
		case GuiItem:		// Generic GuiItem.
			pgui	= new RGuiItem;
			break;
		case Txt:			// RTxt item.
			pgui	= new RTxt;
			break;
		case Btn:			// RBtn item.
			pgui	= new RBtn;
			break;
		case Edit:			// REdit item.
			pgui	= new REdit;
			break;
		case ScrollBar:	// RScrollBar item.
			pgui	= new RScrollBar;
			break;
		case Dlg:			// RDlg item.
			pgui	= new RDlg;
			break;
		case ListBox:		// RListBox item.
			pgui	= new RListBox;
			break;
		case PushBtn:		// RPushBtn item.
			pgui	= new RPushBtn;
			break;
		case MultiBtn:		// RMultiBtn item.
			pgui	= new RMultiBtn;
			break;
		default:
			TRACE("CreateGuiItem(): Unknown type %d.\n", type);
			break;
		}

	// If successfully allocated . . .
	if (pgui != NULL)
		{
		// Mark as dynamic.
		pgui->SetProp(DYNAMIC_PROP_KEY, TRUE);
		}

	return pgui;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destroys the specified object utilizing the 'type' to make sure all
// appropriate destruction for that type is done.  This function is
// useful for delete'ing objects that may be descendants of RGuiItem
// when you do not know the type of the object.  Note this only works
// for standard types.
// (static).
//
//////////////////////////////////////////////////////////////////////////////
void RGuiItem::DestroyGuiItem(	// Returns nothing.
	RGuiItem*	pgui)					// Pointer to gui to deallocate.
	{
	ASSERT(pgui != NULL);

	switch (pgui->m_type)
		{
		case GuiItem:		// Generic GuiItem.
			delete pgui;
			break;
		case Txt:			// RTxt item.
			delete (RTxt*)pgui;
			break;
		case Btn:			// RBtn item.
			delete (RBtn*)pgui;
			break;
		case Edit:			// REdit item.
			delete (REdit*)pgui;
			break;
		case ScrollBar:	// RScrollBar item.
			delete (RScrollBar*)pgui;
			break;
		case Dlg:			// RDlg item.
			delete (RDlg*)pgui;
			break;
		case ListBox:		// RListBox item.
			delete (RListBox*)pgui;
			break;
		case PushBtn:		// RPushBtn item.
			delete (RPushBtn*)pgui;
			break;
		case MultiBtn:		// RMultiBtn item.
			delete (RMultiBtn*)pgui;
			break;
		default:
			TRACE("DestroyGuiItem(): Unknown type %d.\n", pgui->m_type);
			TRACE("DestroyGuiItem(): Using RGuiItem delete.\n");
			delete pgui;
			break;
		}
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
