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
// ListBox.H
// 
// History:
//		01/13/97 JMI	Started.
//
//		01/15/97	JMI	Added overrides of base class's Save/LoadChildren() to
//							implement special cases for m_sbVert, Horz, & 
//							m_frmContents.
//
//		01/21/97	JMI	Added ReadMembers() and WriteMembers() overloads to read
//							and write members of this class.
//
//		01/21/97	JMI	Changed m_frmContents (RFrame) to m_lcContents
//							(RListContents).
//
//		02/05/97	JMI	Added m_typeEncapsulator, the type of GUI item that 
//							AddString() adds.
//
//		02/05/97	JMI	Now overrides OnLoseChild() to deselect the child, if
//							it is the currently selected item.
//
//		05/25/97	JMI	Added GetFirst(), GetNext(), and GetPrev().
//
//		07/04/97	JMI	Added IsListItem().
//
//		09/01/97	JMI	Added MakeEncapsulator().
//
//		09/07/97	JMI	Now EnsureVisible() accepts an optional position 
//							preference so you can bias the location of your item.
//
//		09/22/97	JMI	Also, added friend class CListBoxPropPage for GUI 
//							editor.
//
//////////////////////////////////////////////////////////////////////////////
//
// See CPP for description.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef LISTBOX_H
#define LISTBOX_H

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
	#include "ORANGE/GUI/guiItem.h"
	#include "ORANGE/GUI/scrollbar.h"
	#include "ORANGE/GUI/ListContents.h"
#else
	#include "GuiItem.h"
	#include "ScrollBar.h"
	#include "ListContents.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

// Prop indicating this is an ecapsulator item that should be freed when
// done.
#define ENCAPSULATOR_PROP_KEY	8817473

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
class RListBox : public RGuiItem
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RListBox(void);
		// Destructor.
		~RListBox(void);

	public:	// Typedefs.

		////////////////////////////////////////////////////////////////////////
		// Typedefs.
		////////////////////////////////////////////////////////////////////////

		typedef enum	// State defining visibility of listbox scrollbars.
			{
			Hidden,			// The scrollbar will remain hidden.
			Shown,			// The scrollbar will remain shown.
			ShownAsNeeded	// The scrollbar will be shown when necessary and hidden
								// when not necessary.
			} ScrollBarVisibility;

		typedef enum
			{
			Top,				// Toward the top of the listbox.
			Middle,			// Toward the middle of the listbox.
			Bottom			// Toward the bottom of the listbox.
			} Position;


//////////////////////////////////////////////////////////////////////////////

	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// Methods.
		////////////////////////////////////////////////////////////////////////

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
		virtual						// If you override this, call this base if possible.
			void Destroy(void)	// Returns nothing.
			{
			// Call built-in children.
			m_sbVert.Destroy();
			m_sbHorz.Destroy();
			m_lcContents.Destroy();

			// Call base class.
			RGuiItem::Destroy();
			}

		// Compose item.
		virtual					// If you override this, call this base if possible.
		void Compose(			// Returns nothing.
			RImage* pim = NULL);	// Dest image, uses m_im if NULL.

		// Add a string into the list box.
		// Note that adding a string at the end is by far the fastest.
		RGuiItem* AddString(					// Returns new GUI item on success.
			char*	pszString,					// String to add.
			RGuiItem* pguiAfter	= NULL);	// Gui to add after or NULL to add at
													// end.

		// Insert a string into the list box.
		// Note that adding a string at the end is by far the fastest.
		RGuiItem* InsertString(				// Returns new GUI item on success.
			char*	pszString,					// String to insert.
			RGuiItem* pguiBefore	= NULL);	// Gui to insert before or NULL to 
													// insert at beginning.

		// Add an item into the list box.
		// Note that adding an item at the end is by far the fastest.
		RGuiItem* AddItem(					// Returns new GUI item or pgui on
													// success.  Depends on sEncapsulate.
			RGuiItem* pgui,					// GUI item to insert.
			short	sEncapsulate = FALSE,	// If TRUE, this item will be 
													// encapsulated in an RGuiItem that
													// will be returned on success.
													// If FALSE, this item will be a direct
													// child of the listbox and will be
													// returned on success.
			RGuiItem* pguiAfter	= NULL);	// Gui to add after or NULL to add at
													// end.

		// Note that adding an item at the end is by far the fastest.
		// Insert an item into the list box.
		RGuiItem* InsertItem(				// Returns new GUI item or pgui on
													// success.  Depends on sEncapsulate.
			RGuiItem* pgui,					// GUI item to insert.
			short	sEncapsulate = FALSE,	// If TRUE, this item will be 
													// encapsulated in an RGuiItem that
													// will be returned on success.
													// If FALSE, this item will be a direct
													// child of the listbox and will be
													// returned on success.
			RGuiItem* pguiBefore	= NULL);	// Gui to insert before or NULL to 
													// insert at beginning.

		// Remove an item from the list box and destroy encapsulator, if an
		// encapsulator exists for that item.
		void RemoveItem(						// Returns nothing.
			RGuiItem* pgui);					// Item to remove.  NOTE:  Any 
													// encapsulating GUI item will be
													// destroyed.

		// Remove all items and encapsulators in listbox.
		// Calls RemoveItem() for each item.
		void RemoveAll(void);				// Returns nothing.

		// Ensure the specified item is within the client of the listbox.
		void EnsureVisible(						// Returns nothing.
			RGuiItem* pgui,						// Item to ensure visibility of.
			Position	posPreference = Top);	// In:  Preferred position.

		// Set the selection to the specified GUI item.
		void SetSel(							// Returns nothing.
			RGuiItem* pgui);					// Item to select or NULL for none.

		// Set scroll visibility state for vertical scrollbar.
		void SetVScrollVisibility(			// Returns nothing.
			ScrollBarVisibility	sbv)		// Visibility  for vertical scrollbar
													// (See typedef for ScrollBarVisibility).
			{
			// Set new state.
			m_sbvVert	= sbv;
			// Update visibility.
			UpdateScrollBarVisibility();
			}

		// Set scroll visibility state for horizontal scrollbar.
		void SetHScrollVisibility(			// Returns nothing.
			ScrollBarVisibility	sbv)		// Visibility  for horizontal scrollbar
													// (See typedef for ScrollBarVisibility).
			{
			// Set new state.
			m_sbvHorz	= sbv;
			// Update visibility.
			UpdateScrollBarVisibility();
			}

		// Update scrollbar visibility based on current listbox contents and
		// their visibility.
		void UpdateScrollBarVisibility(void);	// Returns nothing.

		// Resize encapsulators to fit the largest encapsulated item or string,
		// reposition all items to appear in the correct order, resize scrollable
		// area (m_guiContents) to fit all listbox items, and update scrollbar 
		// ranges accordingly.
		void AdjustContents(void);					// Returns nothing.

		// Update scrollbars' positions.  This function is called by 
		// AdjustContents() and EnsureVisible(), so there is no need to call it
		// after calling one of those functions.
		void UpdateScrollBars(void);

		// Called by ListContents() when it is losing a child item.
		virtual				// Overridden here.
		void OnLoseChild(				// Returns nothing.
			RGuiItem*	pguiChild)	// Child item we're about to lose.
			{
			// If this is the selected item . . .
			if (pguiChild == m_pguiSel)
				{
				// Unselect.
				SetSel(NULL);
				}
			}

		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

		// Get the currently selected GUI item.
		RGuiItem* GetSel(void)				// Returns currently selected GUI item
													// or NULL, if none.
			{
			return m_pguiSel;
			}

		// Get the first child item.
		RGuiItem* GetFirst(void);		// Returns the first child item or NULL.

		// Get the child item after the specified item.
		RGuiItem* GetNext(				// Returns the next child item or NULL.
			RGuiItem*	pgui);			// In:  Child item that precedes the next.

		// Get the child item before the specified item.
		RGuiItem* GetPrev(				// Returns the previous child item or NULL.
			RGuiItem*	pgui);			// In:  Child item that follows the prev.

		// Returns TRUE, if the specified item is a list item contained within
		// this listbox; FALSE otherwise.
		short IsListItem(					
			RGuiItem*	pguiListItem)	// In:  Item to check.
			{
			return m_lcContents.IsChild(pguiListItem);
			}


//////////////////////////////////////////////////////////////////////////////

	public:	// Static.

		// Determines if the specified item is an encapsulator item.
		static short IsEncapsulator(	// Returns TRUE, if encapsulator item;
												// FALSE, otherwise.
			RGuiItem*	pgui)				// In:  Item in question.
			{
			// Return encapsulation status.
			return (short)pgui->IsProp(ENCAPSULATOR_PROP_KEY);
			}

		// Makes the specified item an encapsulator item.
		static void MakeEncapsulator(	// Returns nothing.
			RGuiItem*	pgui)				// In:  Item to make an encapsulator item.
			{
			pgui->SetProp(ENCAPSULATOR_PROP_KEY, TRUE);
			}

		// Callback from child items that calls instantiated version.
		static void PressedCall(	// Returns nothing.
			RGuiItem*	pgui)			// Gui item pressed.
			{
			// Call RListBox instantiated.
			((RListBox*)(pgui->m_ulUserInstance))->SetSel(pgui);
			}

		// Callback from scrollbar telling us to update list position.
		static void ScrollCall(		// Returns nothing.
			RScrollBar*	psb)			// Scroll bar.
			{
			RListBox*	plb	= (RListBox*)psb->m_ulUserInstance;
			ASSERT(plb != NULL);

			short	sPos	= psb->GetPos();

			if (psb->m_oOrientation == RScrollBar::Vertical)
				{
				// Move vertically.
				plb->m_lcContents.Move(
					plb->m_lcContents.m_sX,
					-sPos);
				}
			else
				{
				// Move horizontally.
				plb->m_lcContents.Move(
					-sPos,
					plb->m_lcContents.m_sY);
				}
			}

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

		// Creates an item appropriate for the specified string and makes it
		// a child of this listbox.
		RGuiItem* CreateStringItem(	// Returns new item on success; NULL,
												// otherwise.
			char* pszString);				// Text for new item.

		// Creates an encapsulator object for the specified GUI.
		RGuiItem* CreateEncapsulator(	// Returns new item on success; NULL,
												// otherwise.
			RGuiItem*	pgui);			// Item to encapsulate.

		// Place item at specified location in list of container's child items.
		// Under new pretenses, this should not fail.
		void AddAfter(						// Returns nothing.
			RGuiItem*	pgui,				// Item to add.
			RGuiItem*	pguiAfter);		// Item to add after or NULL to add at
												// end.

		// Place item at specified location in list of container's child items.
		// Under new pretenses, this should not fail.
		void InsertBefore(				// Returns nothing.
			RGuiItem*	pgui,				// Item to insert.
			RGuiItem*	pguiBefore);	// Item to insert before or NULL to insert
												// at beginning.

		// [Un]Select the specified item.
		void SelectItem(					// Returns nothing.
			RGuiItem*	pguiSel,			// Item to [un]select.
			short			sSelect);		// If TRUE, item is selected; if FALSE,
												// item is unselected.

		// Save item's children to the specified file.
		virtual					// Overridden here.
		short SaveChildren(	// Returns 0 on success.
			RFile*	pfile);	// File to save to.

		// Load item's children from the specified file.
		virtual					// Overridden here.
		short LoadChildren(	// Returns 0 on success.
			RFile*	pfile);	// File to load from.

		// Read item's members from file.
		virtual				// Overridden here.
		short ReadMembers(			// Returns 0 on success.
			RFile*	pfile,			// File to read from.
			U32		u32Version);	// File format version to use.

		// Write item's members to file.
		virtual				// Overridden here.
		short WriteMembers(			// Returns 0 on success.
			RFile*	pfile);			// File to write to.

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.

		// The scrollable region that owns the list items.
		RListContents			m_lcContents;
		// The vertical scrollbar.
		RScrollBar				m_sbVert;
		// The horizontal scrollbar.
		RScrollBar				m_sbHorz;
		
		// The currently selected item or NULL.
		RGuiItem*				m_pguiSel;

		// Width of largest list item currently in listbox.
		short						m_sLargestWidth;

		// Scrollbar visibility state for vertical scrollbar.
		ScrollBarVisibility	m_sbvVert;
		// Scrollbar visibility state for horizontal scrollbar.
		ScrollBarVisibility	m_sbvHorz;

		// Type of GUI item added via AddString.
		Type						m_typeEncapsulator;

	protected:	// Internal typedefs.

	protected:	// Protected member variables.

	///////////////////////////////////////////////////////////////////////////
	// Friends.
	///////////////////////////////////////////////////////////////////////////
	friend class CListBoxPropPage;

	};

#endif // LISTBOX_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
