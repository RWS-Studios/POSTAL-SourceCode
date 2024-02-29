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
// GuiItem.H
// 
// History:
//		01/13/97 JMI	Started tracking history of this file.
//							Added ListBox to Type enum.
//							Added CopyParms().
//
//		01/14/97	JMI	Added IsDynamic().
//
//		01/16/97	JMI	Added IsClicked(), SetClicked(), and m_sClicked.
//
//		01/18/97	JMI	Added static DoFocus() to handle simple input focus. Also,
//							converted Do() to taking an RInputEvent* instead of a
//							long*.
//
//		01/20/97	JMI	Added GetItemFromPoint() to get a GUI from the tree that
//							contains the specified point.
//
//		01/21/97	JMI	Added static array of strings describing types.
//
//		01/23/97	JMI	Added new member m_sFocusPos which dictates where 
//							(relative to the client x, y) to draw the focus
//							rectangle.
//							Added this functionality to inline DrawFocus().
//							Also, changed SetEventArea() so it includes more than
//							just the client by default.  
//
//		01/26/97	JMI	Altered static HotCall to accept an RHot* instead of a
//							ULONG 'as per' new RHot callbacks.
//
//		02/05/97	JMI	Added enum for new PushBtn (RPushBtn).
//
//		02/05/97	JMI	Now SetParent() calls OnLoseChild() for parent losing
//							child and OnGainChild() for parent gaining child.
//
//		03/10/97	JMI	Added TopPosToClient() and ClientPosToTop().
//
//		03/13/97	JMI	Added SetText(long lId, char* pszFrmt, ...);
//							Added GetVal(long lId) and GetText(long lId, char*);
//
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//		04/01/97	JMI	Changed short m_sCanBeFocused (TRUE, FALSE) to 
//							m_targetFocus (Self, Parent, Sibling).
//							Also, now FocusNext() and FocusPrev() will only focus
//							items that are activated (i.e., IsActivated() != FALSE).
//
//		04/10/97	JMI	Added components for background resource.
//							Added font cell height member.
//
//		04/10/97	JMI	Added enum MultiBtn for RMultiBtn.
//
//		04/24/97	JMI	Added m_u32TextShadowColor.
//
//		06/08/97 MJR	Removed tests for NULL pointers from Destroy().  This
//							was originally intended to avoid getting an obnoxious
//							TRACE message from RImage and RPal.  The problem was
//							that the RImage's special data was not getting freed
//							as a result of this test.  The solution was to get rid
//							of the tests as well as the pointless TRACE messages
//							in RImage and RPal.
//
//		06/30/97 MJR	Moved SetVisible() into .cpp because the declaration of
//							a static inside of it was causing problems with mac
//							precompiled headers.
//
//					MJR	Added RSP_SAFE_GUI_REF() and RSP_SAFE_GUI_REF_VOID().
//
//		07/01/97	JMI	Moved a bunch of functions (some that cannot be
//							inlined (b/c they are virtual) ) into the guiItem.cpp.
//
//		07/04/97	JMI	Added IsChild().
//
//		07/07/97	JMI	Added TextEffectsFlags, m_sTextShadowOffsetX, 
//							and m_sTextShadowOffsetY.
//							Also, added SetTextEffects().
//
//		09/12/97	JMI	Made ReadMembers() and WriteMembers() public.
//
//					JMI	Added GetCurrentFileVersion().
//
//		10/06/99	JMI	Added m_fnInputEvent.
//
//////////////////////////////////////////////////////////////////////////////
//
// See CPP for description.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef GUIITEM_H
#define GUIITEM_H

//////////////////////////////////////////////////////////////////////////////
// Please see the CPP file for an explanation of this API.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////

#include "System.h"
// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "ORANGE/CDT/List.h"
	#include "GREEN/BLiT/BLIT.H"
	#include "GREEN/Hot/hot.h"
	#include "ORANGE/Props/Props.h"
	#include "ORANGE/File/file.h"
	#include "CYAN/cyan.h"
#else
	#include "list.h"
	#include "Image.h"
	#include "BLIT.H"
	#include "hot.h"
	#include "props.h"
	#include "file.h"
	#include "cyan.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define GUI_MAX_STR	1024

#define RSP_SAFE_GUI_REF_VOID(pgui, ref)	((pgui) ? (pgui)->ref : (void)0)
#define RSP_SAFE_GUI_REF(pgui, ref)			((pgui) ? (pgui)->ref : 0)


//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
class RGuiItem : public RProps <U32, U32>
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RGuiItem(void);
		// Destructor.
		virtual ~RGuiItem(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Typedefs/Enums.
		typedef enum		// Values for member flags.
			{					
			// Values for m_sJustification.
			Left,				// Left justified.
			Centered,		// Centered.
			Justified,		// Justified.
			Right,			// Right justified.
			} Justification;

		typedef enum
			{
			// Values for m_sTextEffects.
			Shadow		= 0x0001,	// Shadows text by m_sTextShadowOffsetX, m_sTextShadowOffsetY.
			Bold			= 0x0002,	// Bold text (NYI).
			Italic		= 0x0004,	// Italicized text (NYI).
			ThreeD		= 0x0008,	// 3D (NYI; NSIEWB (not sure if ever will be)).

			} TextEffectsFlags;

		typedef enum	// This type's value indicates a RGuiItem's or descendant's
							// type.
			{
			GuiItem,		// Generic GuiItem.
			Txt,			// RTxt item.
			Btn,			// RBtn item.
			Edit,			// REdit item.
			ScrollBar,	// RScrollBar item.
			Dlg,			// RDlg item.
			ListBox,		// RListBox item.
			PushBtn,		// RPushBtn item.
			MultiBtn,	// RMultiBtn item.
			
			// Insert new type ABOVE this line.
			// Note that Metrowerks requires that there be no comma on the last
			// entry of an enum definition.
			NumGuiTypes	// Placeholder and number of types.
			} Type;

		typedef enum	// Target.
			{
			// Note that Parent and Self are assigned values that
			// aid in backward compatability with older file versions.
			Parent	= FALSE,	// Target this item's parent for action.
			Self		= TRUE,	// Target this item for action.
			Sibling,				// Target a sibling of this item.
			Child,				// Target first child of this item.

			// Insert new targets ABOVE this line.
			NumTargets	// Placeholder and number of targets.
			} Target;

		typedef enum	// Placement type.
			{
			Center	= 0x01,	// Center item or center tiled block of item.
			Tile		= 0x02,	// Tile item.

			// Insert new 
			} Placement;

		// User callback to draw region.
		typedef void (*DrawCall)(	// Returns nothing.
			RGuiItem* pgui,			// The RGuiItem being drawn (this).
			RImage* pim,				// Data to draw; if NULL, erase rect in prc.
			RRect* prc);				// Where/amount to draw/erase in blue coords.

		// User callback to draw background.
		typedef void (*BackCall)(	// Returns nothing.
			RGuiItem* pgui,			// The RGuiItem being composed (this).
			RImage* pim,				// Image to draw into.  Try to stay within 
											// prc please.
			RRect* prc);				// Where to in image.

		// User callback on btn released within button.
		typedef void (*BtnUpCall)(	// Returns nothing.  Called on button released in
											// m_hot when active.
			RGuiItem* pgui);			// this.

		// User callback on any input event directed to GUI via it's RHot (m_hot).
		typedef void (*InputEventCall)(	// Returns nothing.
			RGuiItem*		pgui,				// In:  this.
			RInputEvent*	pie);				// In:  Input event that motivated callback.
													// Out: Typically pie->sUsed = TRUE if used.
													// NOTE: sPosX and sPosY are already mapped
													// into pgui's coordinate system.

		// User callback to get a resource.
		typedef short (*GetResCall)(		// Returns 0 on success; non-zero on failure.
			RGuiItem* pgui);					// this.

		// User callback to release a resource.
		typedef void (*ReleaseResCall)(	// Returns nothing.
			RGuiItem* pgui);					// this.


	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// General.
		////////////////////////////////////////////////////////////////////////

		// Set font utilized for text.  For now, you must set this, but 
		// eventually I hope to have a built in default font embedded into the
		// library so we can display text even when we can't find our assets.
		void SetFont(RFont* pfnt, short sHeight)
			{ m_pprint->SetFont(sHeight, pfnt); }

		// Set the text that represents this item.
		void SetText(	
			char* pszFrmt,	// sprintf formatted format string.
			...);				// Corresponding good stuff.

		// Set the text that represents the specified child item.
		short SetText(		// Returns 0 if item found, non-zero otherwise.
			long	lId,		// Child item ID (can identify this item).
			char* pszFrmt,	// sprintf formatted format string.
			...);				// Corresponding good stuff.

		// Set the justification of m_pprint to the current RGuiItem member 
		// setting.
		void SetJustification(void)	// Returns nothing.
			{
			switch (m_justification)
				{
				case Left:
					m_pprint->SetJustifyLeft();
					break;
				case Centered:
					m_pprint->SetJustifyCenter();
					break;
				case Right:
					m_pprint->SetJustifyRight();
					break;
				default:
					TRACE("SetJustification(): Invalid m_justification value.\n");
					break;
				}
			}

		// Sets up the current text effects on m_pprint to match this
		// GUI's settings.
		void SetTextEffects(void);	// Returns nothing.

		// Creates a displayable Gui.  Call SetFont and SetText before calling
		// this as it calls Compose.
		virtual					// If you override this, call this base if possible.
		short Create(			// Returns 0 on success.
			short sX,			// X position relative to "parent" item.
			short sY,			// Y position relative to "parent" item.
			short sW,			// Width.
			short sH,			// Height.
			short sDepth);		// Color depth.

		// Destroys dynamic display data.
		virtual					// If you override this, call this base if possible.
		void Destroy(void);	// Returns nothing.

		// Draw this item and all its subitems into the provided RImage.
		virtual						// If you override this, call this base if possible.
		short Draw(					// Returns 0 on success.
			RImage* pimDst,		// Destination image.
			short sDstX	= 0,		// X position in destination.
			short sDstY	= 0,		// Y position in destination.
			short sSrcX = 0,		// X position in source.
			short sSrcY = 0,		// Y position in source.
			short sW = 0,			// Amount to draw.
			short sH = 0,			// Amount to draw.
			RRect* prc = NULL);	// Clip to.

		// Pass up a message to redraw the specified region.
		virtual					// If you override this, call this base if possible.
		short Redraw(			// Returns 0 on success.
			short	sSrcX = 0,	// X position to start drawing from.
			short	sSrcY = 0,	// Y position to start drawing from.
			short sW = 0,		// Amount to draw.
			short sH = 0);		// Amount to draw.

		// Blit this item only into provided RImage.  Used by Draw().
		virtual						// If you override this, call this base if possible.
		short Blit(					// Returns 0 on success.
			RImage* pimDst,		// Destination image.
			short sDstX,			// X position in destination.
			short sDstY,			// Y position in destination.
			short sSrcX = 0,		// X position in source.
			short sSrcY = 0,		// Y position in source.
			short sW = 0,			// Amount to draw.
			short sH = 0,			// Amount to draw.
			RRect* prc = NULL);	// Clip to.

		// Draw text in m_szText in m_u32TextColor with transparent
		// background at sX, sY with sW and m_sJustification.
		// Does nothing if m_szText is empty.
		virtual						// If you override this, call this base if possible.
		short DrawText(			// Returns 0 on success.
			short sX,				// X position in image.
			short sY,				// Y position in image.
			short sW = 0,			// Width of text area.
			short	sH = 0,			// Height of test area.
			RImage* pim = NULL);	// Destination image.  NULL == use m_im.

		// Ask user to erase area specified.
		virtual					// If you override this, call this base if possible.
		void Erase(				// Returns nothing.
			short sX = 0,		// X position to erase.
			short sY = 0,		// Y position to erase.
			short sW = 0,		// Width to erase.
			short sH = 0);		// Height to erase.

		// Move this item to sX, sY.
		virtual					// If you override this, call this base if possible.
		void Move(				// Returns nothing.
			short sX,			// New x position.
			short sY);			// New y position.

		// Cursor event notification.
		// Events in event area.
		virtual						// If you override this, call this base if possible.
		void CursorEvent(			// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Callback from RHot.
		// Events in hot area.
		virtual						// If you override this, call this base if possible.
		void HotCall(				// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Activate or deactivate mouse reaction for this gui item only.
		// If the item has m_sActive set to TRUE, it will be activated.
		virtual					// If you override this, call this base if possible.
		void SetActive(		// Returns nothing.
			short sActive);	// TRUE to make active, FALSE otherwise.

		// Hide or show GUI item and child items.  Causes RHots to be
		// deactivated for both this and child items.
		// DOES NOT CHANGE THE VISIBLE STATUS OF CHILD ITEMS.
		virtual					// If you override this, call this base if possible.
		void SetVisible(		// Returns nothing.
			short sVisible);	// TRUE to make visible, FALSE otherwise.

		// Change parent.  Removes from old parent's list and adds to new.
		// New can be NULL (so can old).
		virtual					// If you override this, call this base if possible.
		void SetParent(RGuiItem* pguiParent);

		// Draw border.
		virtual					// If you override this, call this base if possible.
		void DrawBorder(		// Returns nothing.
			RImage* pim	= NULL,			// Dest image, uses m_im if NULL.
			short sInvert	= FALSE);	// Inverts border if TRUE.

		// Draw background.  Calls user callback m_backcall if provided.
		virtual					// If you override this, call this base if possible.
		void DrawBackground(			// Returns nothing.
			RImage* pim	= NULL);		// Dest image, uses m_im if NULL.

		// Draw background resource, if one is specified.
		// Utilizes m_*BkdRes* parameters to get, place, and BLiT the resource.
		virtual					// If you override this, call this base if possible.
		void DrawBackgroundRes(		// Returns nothing.
			RImage* pim = NULL);		// Dest image, uses m_im, if NULL.

		// Compose item.
		virtual					// If you override this, call this base if possible.
		void Compose(			// Returns nothing.
			RImage* pim = NULL);	// Dest image, uses m_im if NULL.

		// Do one iteration of processing.  This is useful on items that need to
		// poll frequently or do something every once in a while that is not
		// triggered by an event.
		virtual						// If you override this, call this base if possible.
		void Do(						// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Set this item's event area.  This is the area where cursor events are
		// interesting to the item.
		virtual							// If you override this, call this base if possible.
		void SetEventArea(void);	// Returns nothing.

		// Set this item's hot area.  This should be the total dimensions of this
		// item.
		virtual						// If you override this, call this base if possible.
		void SetHotArea(void);	// Returns nothing.

		// Copy basic parameters regarding appearance and use from this item to 
		// the specified one.
		virtual						// If you override this, call this base if possible.
		void CopyParms(			// Returns nothing.
			RGuiItem* pguiDst);	// Destination for parameters from this item.

		// Called by SetParent() when a GUI is losing a child item.
		virtual				// If you override this, call this base, if possible.
		void OnLoseChild(				// Returns nothing.
			RGuiItem*	pguiChild);	// Child item we're about to lose.

		// Called by SetParent() when a GUI is gaining a child item.
		virtual				// If you override this, call this base, if possible.
		void OnGainChild(				// Returns nothing.
			RGuiItem*	pguiChild);	// Child item we're about to gain.

		// This will open the specified file with read access in an RFile and
		// pass it to Load(RFile*).
		// NOTE:  There is a static version of this function that will detect
		// the type of the root item from the file and allocate an item of
		// that type before loading.  Useful for loading generic *.GUI files.
		// See LoadInstance(char*).
		short Load(					// Returns 0 on success.
			char* pszFileName);	// Name of file to load from.

		// This will load the GUI tree (i.e., 'this' item and its children).
		// If you override this function, you should call this base version
		// to have it load the base class members.
		// NOTE:  There is a static version of this function that will detect
		// the type of the root item from the file and allocate an item of
		// that type before loading.  Useful for loading generic *.GUI files.
		// See LoadInstance(RFile*).
		virtual						// If you override this, call this base if possible.
		short Load(					// Returns 0 on success.
			RFile*	pfile);		// RFile open with read access to load from.

		// This will open the specified file with write access in an RFile and
		// pass it to Save(RFile*).
		short Save(					// Returns 0 on success.
			char* pszFileName);	// Name of file to save to.

		// This will save the GUI tree (i.e., 'this' item and its children).
		// If you override this function, you should call this base version
		// to have it save the base class members.
		virtual						// If you override this, call this base if possible.
		short Save(					// Returns 0 on success.
			RFile*	pfile);		// RFile open with write access to save to.

		// Sets the 'clicked' status.
		void SetClicked(			// Returns nothing.
			short sClicked);		// New 'clicked' status.

		// Draws focus for item if m_sShowFocus is TRUE.
		virtual						// If you override this, call this base if possible.
		void DrawFocus(			// Returns nothing.
			RImage* pimDst,		// Destination image.
			short sDstX	= 0,		// X offset in destination.
			short sDstY	= 0,		// Y offset in destination.
			RRect* prc = NULL);	// Clip to.

		// Called by the static implementation of SetFocus() on the item gaining
		// the focus.
		// It is okay to call SetFocus() from this function.
		virtual				// If you override this, call this base if possible.
		void OnGainFocus(void);

		// Called by the static implementation of SetFocus() on the item losing
		// the focus.
		// It is okay to call SetFocus() from this function.
		virtual				// If you override this, call this base if possible.
		void OnLoseFocus(void);

		// Sets the current focus GUI pointer to this GUI.
		RGuiItem* SetFocus(void);	// Returns pointer to GUI losing the focus.

		// Gets the optional resource image named m_szBkdResName for this item 
		// into m_pimBkdRes.
		// Each call to this function should have a corresponding call to
		// ReleaseRes().
		// This function can result in a callback.
		virtual				// If you override this, call this base if possible.
		short GetRes(void);

		// Releases the optional resource image previously gotten by a call to
		// GetRes().
		// Each call to this function should correspond to a previous call to
		// GetRes().  You should not call this function if the corresponding
		// GetRes() failed.
		// This function can result in a callback.
		virtual				// If you override this, call this base if possible.
		void ReleaseRes(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Static

		// Sets the current focus GUI pointer to the supplied GUI.
		// NOTE:  Eventually, this function may do more, so it is better to call
		// it than simply setting ms_pguiFocus to your gui.
		static RGuiItem* SetFocus(		// Returns pointer to GUI losing the focus.
			RGuiItem*	pguiNewFocus);	// New item to focus or NULL for none.

		// Moves the focus to the next GUI at the same level as the current
		// ms_pguiFocus.  Does not affect current focus for top-level guis.
		static RGuiItem* FocusNext(void);		// Returns new item with focus or NULL,
															// if none.

		// Moves the focus to the previous GUI at the same level as the current
		// ms_pguiFocus.  Does not affect current focus for top-level guis.
		static RGuiItem* FocusPrev(void);		// Returns new item with focus or NULL,
															// if none.

		// Sends the event to the GUI focused via Do(), and, if event is not 
		// used, checks for  keys that might change the focus.
		static void DoFocus(		// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Creates the specified object and returns a pointer to it.
		// The object is created with new, so use delete to destroy the object.
		static RGuiItem* CreateGuiItem(	// Returns the allocated type on success.
													// Returns NULL on error.
			Type	type);						// Type of GuiItem to allocate.  Must be one
													// of the enums that is a member of 
													// RGuiItem::Type.

		// Destroys the specified object utilizing the 'type' to make sure all
		// appropriate destruction for that type is done.  This function is
		// useful for delete'ing objects that may be descendants of RGuiItem
		// when you do not know the type of the object.  Note this only works
		// for standard types.
		static void DestroyGuiItem(		// Returns nothing.
			RGuiItem*	pgui);				// Pointer to gui to deallocate.

		// Instantiate a GUI tree from a file.  Allocates a GUI item of the
		// proper (specified in the file) type and loads it using the specified
		// file.
		static RGuiItem* LoadInstantiate(	// Returns newly allocated GUI item
														// on success or NULL on failure.
			char*	pszFileName);					// Name of file to instantiate from.

		// Instantiate a GUI tree from a file.  Allocates a GUI item of the
		// proper (specified in the file) type and loads it using the specified
		// file.
		static RGuiItem* LoadInstantiate(	// Returns newly allocated GUI item
														// on success or NULL on failure.
			RFile*	pfile);						// Pointer to open GUI file.

		// Read item's members from file.
		virtual				// If you override this, call this base if possible.
		short ReadMembers(			// Returns 0 on success.
			RFile*	pfile,			// File to read from.
			U32		u32Version);	// File format version to use.

		// Write item's members to file.
		virtual				// If you override this, call this base if possible.
		short WriteMembers(			// Returns 0 on success.
			RFile*	pfile);			// File to write to.

	public:	
		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

		// Get the text that represents this item.
		short GetText(		// Returns 0 on success.
			char* pszText,	// Location to copy text to.
			short sMax);	// Total memory pointed to by pszText.

		// Get the text that represents the specified item.
		short GetText(		// Returns 0 on success.
			long	lId,		// In:  Child item ID (can identify this item). 
			char* pszText,	// Out: Location to copy text to.
			short sMax);	// In:  Total memory pointed to by pszText.

		// Get the number represented by the text in this item.
		long GetVal(void);		// Returns value.

		// Get the number represented by the text in the specified item.
		long GetVal(		// Returns value.
			long	lId);		// In:  Child item ID (can identify this item). 

		// Get the RImage that contains the item.  Feel free to Convert() this
		// to even a transparent type.  Blah ha ha.
		RImage* GetImage(void)	// Returns a pointer to this dialog's RImage.
			{ return &m_im; }

		// Get the parent of this gui item.
		RGuiItem* GetParent(void)	// Returns parent RGuiItem*.
			{ return m_pguiParent; }

		// Gets the thickness of the top/left border (including border edge effect).
		virtual
		short GetTopLeftBorderThickness(void);	// Returns border thickness 
															// including edge effect.                      

		// Gets the thickness of the bottom/right border (including border edge effect).
		virtual
		short GetBottomRightBorderThickness(void);	// Returns border thickness 
																	// including edge effect.                      

		// Get the "client" area (i.e., non-border/title area) relative to this
		// item.
		virtual					// If you override this, call this base if possible.
		void GetClient(		// Returns nothing.
			short* psX,			// Out: X position unless NULL.
			short* psY,			// Out: Y position unless NULL.
			short* psW,			// Out: Width unless NULL.
			short* psH);		// Out: Height unless NULL.

		// Get the "hot" area (i.e., clickable area) relative to this item.
		virtual					// If you override this, call this base if possible.
		void GetHotArea(		// Returns nothing.
			short* psX,			// Out: X position unless NULL.
			short* psY,			// Out: Y position unless NULL.
			short* psW,			// Out: Width unless NULL.
			short* psH);		// Out: Height unless NULL.

		// Change the position specified to a top-level coord.
		void ChildPosToTop(	// Returns nothing.
			short* psX,			// In: Child pos, Out: Top level pos.
			short* psY);		// In: Child pos, Out: Top level pos.

		// Change the position specified to a child coord.
		void TopPosToChild(	// Returns nothing.
			short* psX,			// In: Top level pos, Out: Child pos.
			short* psY);		// In: Top level pos, Out: Child pos.

		// Change the position specified to a top-level coord.
		void ClientPosToTop(	// Returns nothing.
			short* psX,			// In: Client pos, Out: Top level pos.
			short* psY)			// In: Client pos, Out: Top level pos.
			{
			// Get client position.
			short	sClientX, sClientY;
			GetClient(&sClientX, &sClientY, NULL, NULL);
			// Offset.
			*psX	+= sClientX;
			*psY	+= sClientY;
			// Convert to top.
			ChildPosToTop(psX, psY);
			}

		// Change the position specified to a client coord.
		void TopPosToClient(	// Returns nothing.
			short* psX,			// In: Top level pos, Out: Client pos.
			short* psY)			// In: Top level pos, Out: Client pos.
			{
			// Convert to child.
			TopPosToChild(psX, psY);
			// Get client position.
			short	sClientX, sClientY;
			GetClient(&sClientX, &sClientY, NULL, NULL);
			// Offset.
			*psX	-= sClientX;
			*psY	-= sClientY;
			}

		// Get the first item with the supplied ID from this among RGuiItem and
		// its children.
		RGuiItem* GetItemFromId(	// Returns pointer to RGuiItem, if found;
											// otherwise, returns NULL.
			long lId)					// ID of RGuiItem to find.
			{
			RGuiItem*	pguiRes	= NULL;	// Assume not found.

			// First check this item . . .
			if (m_lId != lId)
				{
				// Check children.
				RGuiItem*	pgui	= m_listguiChildren.GetHead();
				while (pgui != NULL && pguiRes == NULL)
					{
					// Check pgui and its children.
					pguiRes	= pgui->GetItemFromId(lId);

					pgui		= m_listguiChildren.GetNext();
					}
				}
			else
				{
				// This item is it.
				pguiRes	= this;
				}

			return pguiRes;
			}

		// Get the item at the specified point that has the
		// specified parameters.
		RGuiItem* GetItemFromPoint(	// Returns item ptr, if item found;
												// NULL on failure.
			short	sPosX,					// X position.
			short	sPosY,					// Y position.
			short	sActive = TRUE,		// If TRUE, only searches active items.
												// If FALSE, searches all items.
			short sEventArea = TRUE);	// If TRUE, only checks items' event areas.
												// If FALSE, checks items' entire hot regions.

		// Gets the child depth (i.e., how many ancestors until top-level).
		short GetChildDepth(void)	// Returns child depth.
			{
			short	sDepth	= 0;

			RGuiItem*	pguiParent	= GetParent();
			while (pguiParent != NULL)
				{
				// Increase depth.
				sDepth++;
				
				// Get parent of this parent.
				pguiParent	= pguiParent->GetParent();
				}

			return sDepth;
			}

		// Determine if the specified item is an ancestor of this item.
		short IsAncestor(		// Returns TRUE if pgui is an ancestor of this item.
									// Returns FALSE otherwise.
			RGuiItem* pgui)	// GUI that may be an ancestor.
			{
			short	sRes	= FALSE;

			RGuiItem*	pguiParent =	GetParent();
			while (pguiParent != NULL && sRes == FALSE)
				{
				if (pguiParent == pgui)
					{
					// Found item is an ancestor.
					sRes	= TRUE;
					}

				pguiParent =	pguiParent->GetParent();
				}

			return sRes;
			}

		// See if item is currently activated.
		short IsActivated(void)	// Returns TRUE if this item is activated (which
										// is different from active (active items become
										// activated when visible)).
			{
			return m_hot.IsActive();
			}

		// Returns TRUE if this item was allocated with CreateGuiItem().
		short IsDynamic(void);	// Returns TRUE if this item was allocated with
										// CreateGuiItem().

		// Returns TRUE if this item is considered 'Clicked'.  For example, the
		// default implementation makes this TRUE if the cursor was pressed AND
		// released within this item's hot area.
		short IsClicked(void)	// Returns TRUE, if this item was 'Clicked';
										// FALSE, otherwise.
			{
			return m_sClicked;
			}

		// Returns TRUE if the specified item is a direct descendant of
		// this item; FALSE, otherwise.
		// NOTE:  Must be direct descendant; children of children don't count.
		short IsChild(
			RGuiItem* pguiChild)	// In:  Item to check.
			{
			if (pguiChild->GetParent() == this)
				return TRUE;
			return FALSE;
			}

		// Get the current file version (that is, the file version a file
		// written from this GUI right now would be).
		static U32 GetCurrentFileVersion(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Static


//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

		// Pass callbacks from RHot on to appropriate instance.
		static void HotCall(
			RHot*	phot,			// In:  Ptr to RHot that generated event.
			RInputEvent* pie)	// In:  Most recent user input event.             
									// Out: pie->sUsed = TRUE, if used.
			{ ((RGuiItem*)(phot->m_ulUser))->HotCall(pie); }

		// Read header for this GUI item.
		static short ReadHeader(	// Returns 0 on success.
			RFile*	pfile,			// In:  File to read from.
			U32*	pu32Version,		// Out: File format version.
			Type*	ptype);				// Out: Type of GUI item stored.

		// Save item's children to the specified file.
		virtual
		short SaveChildren(	// Returns 0 on success.
			RFile*	pfile);	// File to save to.

		// Load item's children from the specified file.
		virtual
		short LoadChildren(	// Returns 0 on success.
			RFile*	pfile);	// File to load from.

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.
		RImage				m_im;			// RImage representing this item 
												// visually.
		RList<RGuiItem>	m_listguiChildren;	// List of items "inside" this item.
		short					m_sX;			// X position.  0 on construction.
		short					m_sY;			// Y position.  0 on construction.

		RPrint*				m_pprint;	// Used to display text.  Also, 
												// stores font settings.  By default,
												// this points to &ms_print, but it
												// can be changed to point to a dif-
												// ferent one.
		short					m_sFontCellHeight;	// Cell height for text.
		char					m_szText[GUI_MAX_STR];	// Text for this item.

		short					m_sEventAreaX;	// X coord of area in which we care
													// about cursor events.
		short					m_sEventAreaY;	// Y coord of area in which we care
													// about cursor events.
		short					m_sEventAreaW;	// Width of area in which we care
													// about cursor events.
		short					m_sEventAreaH;	// Height of area in which we care
													// about cursor events.

		RHot					m_hot;			// Area in which we get clickage.
		
		short					m_sPressed;	// TRUE if our hot area is currently
												// pressed.

		DrawCall				m_drawcall;	// User callback to draw/erase a region.
		BackCall				m_backcall;	// User callback to draw background in
												// item.
		BtnUpCall			m_bcUser;	// User callback on button up in m_hot
												// when active.

		InputEventCall		m_fnInputEvent;	// User callback when input event
														// is directed at this GUI via its
														// RHot (m_hot).



		U32					m_u32BorderColor;
		U32					m_u32BorderShadowColor;
		U32					m_u32BorderHighlightColor;
		U32					m_u32BorderEdgeColor;

		U32					m_u32TextColor;			// 0 indicates transparency.
		U32					m_u32BackColor;
		U32					m_u32TextShadowColor;

		short					m_sTextEffects;			// Flags for text effects.

		short					m_sTextShadowOffsetX;	// Offset along X axis for text shadow.
		short					m_sTextShadowOffsetY;	// Offset along Y axis for text shadow.

		short					m_sBorderThickness;

		Justification		m_justification;	// { RGuiItem::Right, RGuiItem::Center, RGuiItem::Left }

		ULONG					m_ulUserInstance;	// Space that can be used in any way by 
														// the user but is intended to represent 
														// a user instance structure such as a 
														// struct or class.
		ULONG					m_ulUserData;		// Space that can be used in any way by
														// the user and has no particular intended
														// use.

		short					m_sInvertedBorder;	// TRUE if border is inverted; FALSE 
															// otherwise.

		Type					m_type;				// Indicates type of GUI item.

		short					m_sTransparent;	// TRUE, if this should be blt'ed via
														// a transparent blit call; FALSE,
														// otherwise.  Note that this cannot
														// override transparent Image formats
														// (e.g., FSPR8 will always be blt'ed
														// with transparency).

		U32					m_u32TransparentColor;	// Color used for transparency
																// when using transparent blit
																// call.

		long					m_lId;				// ID.  Used to identify this RGuiItem
														// from others.  See GetItemFromId().

		Target				m_targetFocus;		// Target when focus received.

		short					m_sShowFocus;		// TRUE if this item shows feedback,
														// usually via DrawFocus(), when it
														// has the focus.
		
		short					m_sFocusPos;		// Position at which DrawFocus() will
														// draw the focus rectangle relative
														// to the client area.  For example,
														// -1 would but it just outside the
														// client area.

		U32					m_u32FocusColor;	// Color to draw focus with.

		short					m_sClicked;			// TRUE if this item is considered
														// 'Clicked'.  For example, the default
														// implementation makes this TRUE if
														// the cursor was pressed AND released
														// within this item's hot area.

		char					m_szBkdResName[RSP_MAX_PATH];	// Name of background res
																		// file to get into 
																		// m_pimBkdRes.
		short					m_sBkdResTransparent;	// TRUE, if m_pimBkdRes is to be
																// BLiT'ed transparently; FALSE,
																// otherwise.
		U32					m_u32BkdResTransparentColor;	// Transparency color for
																		// m_pimBkdRes when 
																		// BLiT'ed transparently.
		short					m_sBkdResPlacement;	// Combination of |'ed
															// Placement enum values.
		RImage*				m_pimBkdRes;			// Background resource image.


		GetResCall			m_fnGetRes;			// User callback to get background res
														// (m_pimBkdRes).
		ReleaseResCall		m_fnReleaseRes;	// User callback to release background
														// res (m_pimBkdRes).

		// These members should be changed only via SetVisible() and SetActive().
		short					m_sVisible;			// TRUE if Draw() is to draw this item
														// and its children; FALSE, otherwise.
		short					m_sActive;			// TRUE if the RHot is to be activated
														// when visible.

	protected:	// Internal typedefs.

	protected:	// Protected member variables.
		RGuiItem*			m_pguiParent;

		//////////////////// Statics ////////////////////////////////////////////
	public:
		static RPrint		ms_print;			// This is the main RPrint that all
														// GUI items default to.
		static RGuiItem*	ms_pguiFocus;		// Higher level APIs can use this as
														// their current point of input
														// focus.
		static char*		ms_apszTypes[NumGuiTypes];	// Array of strings 
																	// indexed by type.
	};

#endif // GUIITEM_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
