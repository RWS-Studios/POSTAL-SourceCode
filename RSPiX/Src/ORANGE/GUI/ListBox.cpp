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
// ListBox.CPP
// 
// History:
//		01/13/97 JMI	Started.
//
//		01/15/97	JMI	SetProp() now takes the Key parm first and the Item parm
//							second.  Changed this module to reflect that.
//
//		01/15/97	JMI	Added overrides of base class's Save/LoadChildren() to
//							implement special cases for m_sbVert, Horz, & 
//							m_lcContents.  List items saved and then loaded will not
//							be recognized as list items by this class.  This should
//							eventually be supported so one can save a list of items.
//
//		01/15/97	JMI	Since loading of list items does not work properly, any
//							list items in m_lcContents are destroyed immediately
//							after they are loaded.
//
//		01/20/97	JMI	UpdateScrollBarVisibility() was not working correctly for
//							Hidden.  Fixed.
//
//		01/21/97	JMI	Added ReadMembers() and WriteMembers() overloads to read
//							and write members of this class.  Note that they call the
//							base class version to read/write base class members.
//							Support exists for versions 0 and 1.  Version 0 did not
//							contain other than RGuiItem members.
//
//		01/21/97	JMI	RemoveItem() was not considering the possibly that the
//							item being removed might be the selected item.
//							No longer sizes an encapsulator/string item to 
//							m_sLargestWidth until the AdjustContents() call.
//
//		01/21/97	JMI	Changed m_frmContents (RFrame) to m_lcContents
//							(RListContents).
//
//		01/23/97	JMI	EnsureVisible() had an error that made it think that 
//							some items that were list items were not.
//
//		02/05/97	JMI	Changed position of default: case in ReadMembers().
//							And made CreateStringItem() use m_typeEncapsulator as
//							the type of the created encapsulator.
//							Note that CreateEncapsulator() still only uses RTxts.
//
//		02/25/97	JMI	SaveChildren() now goes through the children in reverse
//							order so they, on load, get added back to their parent in
//							the order they were originally added to this parent.
//
//		03/28/97	JMI	Now AdjustContents() adjusts vertical scroll bar incs
//							so that the buttons will advance one item and the tray
//							will scroll visible items - 1.
//
//		04/10/97	JMI	Now uses m_sFontCellHeight instead of GetPos() to get
//							cell height.
//
//		05/25/97	JMI	Added GetFirst(), GetNext(), and GetPrev().
//
//		06/15/97	JMI	List items are now at least as wide as the viewable area.
//
//		08/11/97	JMI	Seemed like there was a potential for infinite loop in
//							GetNext() and GetPrev().
//							Also, GetFirst() returned the first item in the list
//							w/o checking if it was an encapsulator.
//
//		09/01/97	JMI	Added MakeEncapsulator().
//
//		09/07/97	JMI	Now EnsureVisible() accepts an optional position 
//							preference so you can bias the location of your item.
//
//		09/24/97	JMI	Calls to InsertAfter() and InsertBefore() had their
//							parameters backward.
//
//////////////////////////////////////////////////////////////////////////////
//
// This a GUI item that is based on the basic RGuiItem. 
// This overrides HotCall() to get information about where a click in its RHot
// occurred.
// This overrides Compose() to create the scrollable area and compose 
// the scrollbars.
//
// Enhancements/Uses:
//		Not yet known.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/ListBox.h"
	#include "ORANGE/GUI/txt.h"
#else
	#include "listbox.h"
	#include "txt.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Sets val to def if val is -1.
#define DEF(val, def)	((val == -1) ? def : val)

// Default width for vertical scrollbar and height for horizontal scrollbar.
#define DEF_SCROLL_THICKNESS	13

// Space between items.
#define ITEM_SPACING				m_sBorderThickness

// Scrollbar priority is very high so it will be above other siblings.
#define SCROLLBAR_PRIORITY		((short)0x8001)

// Frame should be lower priority so that scrollbars are 'on top' of it.
#define FRAME_PRIORITY			((short)0x7FFF)

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RListBox::RListBox()
	{
	// Override defaults for this.
	m_type						= ListBox;	// Indicates type of GUI item.
	m_sInvertedBorder			= TRUE;		// Want sunken look.
	
	// Override defaults for children.
	m_sbVert.m_im.m_sWidth		= DEF_SCROLL_THICKNESS;
	m_sbVert.m_oOrientation		= RScrollBar::Vertical;
	m_sbVert.m_upcUser			= ScrollCall;
	m_sbVert.m_ulUserInstance	= (ULONG)this;

	// Priority for scrollbars should be higher than any other sibling.
	m_sbVert.m_hot.SetPriority(SCROLLBAR_PRIORITY);

	m_sbHorz.m_im.m_sHeight		= DEF_SCROLL_THICKNESS;
	m_sbHorz.m_oOrientation		= RScrollBar::Horizontal;
	m_sbHorz.m_upcUser			= ScrollCall;
	m_sbHorz.m_ulUserInstance	= (ULONG)this;

	// Priority for scrollbars should be higher than any other sibling.
	m_sbHorz.m_hot.SetPriority(SCROLLBAR_PRIORITY);

	// Priority for frame is low.
	m_lcContents.m_hot.SetPriority(FRAME_PRIORITY);

	// Set defaults for direct members.
	m_sbvVert					= ShownAsNeeded;
	m_sbvHorz					= ShownAsNeeded;
									
	m_sLargestWidth			= 0;
									
	m_pguiSel					= NULL;
	
	m_typeEncapsulator		= Txt;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RListBox::~RListBox()
	{
	// Remove all items and destroy their encapsulators.
	RemoveAll();
	}

////////////////////////////////////////////////////////////////////////
// Methods.
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Creates a displayable Listbox item.
//
//////////////////////////////////////////////////////////////////////////////
short RListBox::Create(			// Returns 0 on success.
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
	
	// Get client area.
	short	sClientX, sClientY, sClientW, sClientH;
	GetClient(&sClientX, &sClientY, &sClientW, &sClientH);

	// Create child items:

	CopyParms(&m_lcContents);

	m_lcContents.m_sBorderThickness	= 0;

	// Frame is created in AdjustContents() call.
	m_lcContents.SetParent(this);

	CopyParms(&m_sbVert);

	if (m_sbVert.Create(
		sClientX + sClientW - m_sbVert.m_im.m_sWidth,
		sClientY,
		m_sbVert.m_im.m_sWidth,
		sClientH,
		sDepth) == 0)
		{
		m_sbVert.SetParent(this);
		}
	else
		{
		TRACE("Create(): m_sbVert.Create() failed.\n");
		// Don't use this.
		m_sbVert.SetParent(NULL);
		}

	CopyParms(&m_sbHorz);

	if (m_sbHorz.Create(
		sClientX,
		sClientY + sClientH - m_sbHorz.m_im.m_sHeight,
		sClientW - m_sbVert.m_im.m_sWidth,
		m_sbHorz.m_im.m_sHeight,
		sDepth) == 0)
		{
		m_sbHorz.SetParent(this);
		}
	else
		{
		TRACE("Create(): m_sbHorz.Create() failed.\n");
		// Don't use this.
		m_sbHorz.SetParent(NULL);
		}
	
	// Compose this item.
	Compose();

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Compose item.
//
////////////////////////////////////////////////////////////////////////
void RListBox::Compose(			// Returns nothing.
	RImage* pim /*= NULL*/)	// Dest image, uses m_im if NULL.
	{
	if (pim == NULL)
		{
		pim	= &m_im;
		}

	// Call base (draws border and background).
	RGuiItem::Compose(pim);

	// Resizes all encapsulators, m_lcContents, and sets scroll ranges.
	AdjustContents();

	// No text.
	}

////////////////////////////////////////////////////////////////////////
//
// Add a string into the list box.
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RListBox::AddString(		// Returns new GUI item on success.
	char*	pszString,						// String to add.
	RGuiItem* pguiAfter /*= NULL*/)	// Gui to add after or NULL to add at
												// end.
	{
	// Create a new string item . . .
	RGuiItem*	pguiRes	= CreateStringItem(pszString);
	if (pguiRes != NULL)
		{
		// Reposition item.
		AddAfter(pguiRes, pguiAfter);
		}
	else
		{
		TRACE("AddString(): CreateStringItem() failed.\n");
		}

	return pguiRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Insert a string into the list box.
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RListBox::InsertString(	// Returns new GUI item on success.
	char*	pszString,						// String to insert.
	RGuiItem* pguiBefore /*= NULL*/)	// Gui to insert before or NULL to 
												// insert at beginning.
	{
	// Create a new string item . . .
	RGuiItem*	pguiRes	= CreateStringItem(pszString);
	if (pguiRes != NULL)
		{
		// Reposition item.
		InsertBefore(pguiRes, pguiBefore);
		}
	else
		{
		TRACE("InsertString(): CreateStringItem() failed.\n");
		}

	return pguiRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Add an item into the list box.
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RListBox::AddItem(			// Returns new GUI item or pgui on
												// success.  Depends on sEncapsulate.
	RGuiItem* pgui,						// GUI item to insert.
	short	sEncapsulate /*= FALSE*/,	// If TRUE, this item will be 
												// encapsulated in an RGuiItem that
												// will be returned on success.
												// If FALSE, this item will be a direct
												// child of the listbox and will be
												// returned on success.
	RGuiItem* pguiAfter /*= NULL*/)	// Gui to add after or NULL to add at
												// end.
	{
	RGuiItem*	pguiRes	= NULL;	// Assume nothing.

	// If encapsulation requested . . .
	if (sEncapsulate != FALSE)
		{
		// Create encapsulator item.
		pguiRes	= CreateEncapsulator(pgui);
		}
	else
		{
		// The item is the item to be placed in the listbox.
		pguiRes	= pgui;
		}

	// If we have the item to be added . . .
	if (pguiRes != NULL)
		{
		// Make child of list area.
		pguiRes->SetParent(&m_lcContents);
		// Reposition item.
		AddAfter(pguiRes, pguiAfter);
		}
	else
		{
		TRACE("AddItem(): CreateEncapsulator() failed.\n");
		}

	return pguiRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Insert an item into the list box.
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RListBox::InsertItem(		// Returns new GUI item or pgui on
												// success.  Depends on sEncapsulate.
	RGuiItem* pgui,						// GUI item to insert.
	short	sEncapsulate /*= FALSE*/,	// If TRUE, this item will be 
												// encapsulated in an RGuiItem that
												// will be returned on success.
												// If FALSE, this item will be a direct
												// child of the listbox and will be
												// returned on success.
	RGuiItem* pguiBefore /*= NULL*/)	// Gui to insert before or NULL to 
												// insert at beginning.
	{
	RGuiItem*	pguiRes	= NULL;	// Assume nothing.

	// If encapsulation requested . . .
	if (sEncapsulate != FALSE)
		{
		// Create encapsulator item.
		pguiRes	= CreateEncapsulator(pgui);
		}
	else
		{
		// The item is the item to be placed in the listbox.
		pguiRes	= pgui;
		}

	// If we have the item to be added . . .
	if (pguiRes != NULL)
		{
		// Make child of list area.
		pguiRes->SetParent(&m_lcContents);
		// Reposition item.
		InsertBefore(pguiRes, pguiBefore);
		}
	else
		{
		TRACE("AddItem(): CreateEncapsulator() failed.\n");
		}

	return pguiRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Remove an item from the list box and destroy encapsulator, if an
// encapsulator exists for that item.
//
////////////////////////////////////////////////////////////////////////
void RListBox::RemoveItem(			// Returns nothing.
	RGuiItem* pgui)					// Item to remove.  NOTE:  Any 
											// encapsulating GUI item will be
											// destroyed.
	{
	// Verify item is a list item and get encapsulator, if any:
	RGuiItem*	pguiParent	= pgui->GetParent();
	RGuiItem*	pguiRemove	= NULL;
	if (pguiParent != NULL)
		{
		// If parent is the list container . . .
		if (pguiParent == &m_lcContents)
			{
			// Set item to remove.
			pguiRemove	= pgui;
			}
		else
			{
			// If parent's parent is the list container . . .
			if (pguiParent->GetParent() == &m_lcContents)
				{
				// Free from encapsulator.
				pgui->SetParent(NULL);
				// Set item to remove.
				pguiRemove	= pguiParent;
				}
			}
		}

	// If there is an item to remove . . .
	if (pguiRemove != NULL)
		{
		// If this item is an encapsulator . . .
		if (IsEncapsulator(pguiRemove) != FALSE)
			{
			// If this item was the selected item . . .
			if (pguiRemove == m_pguiSel)
				{
				// Clear selection.
				SetSel(NULL);
				}

			// Remove encapsulator prop.
			pguiRemove->RemoveProp(ENCAPSULATOR_PROP_KEY);
			// Destroy item.
			delete pguiRemove;
			}
		else
			{
			// If dynamic . . .
			if (pguiRemove->IsDynamic() != FALSE)
				{
				delete pguiRemove;
				}
			}
		}
	else
		{
		TRACE("RemoveItem():  Specified item is not an item of this listbox.\n");
		}

	}

////////////////////////////////////////////////////////////////////////
//
// Remove all items and encapsulators in listbox.
// Calls RemoveItem() for each item.
//
////////////////////////////////////////////////////////////////////////
void RListBox::RemoveAll(void)	// Returns nothing.
	{
	RGuiItem*	pgui	= m_lcContents.m_listguiChildren.GetHead();
	while (pgui != NULL)
		{
		RemoveItem(pgui);

		pgui	= m_lcContents.m_listguiChildren.GetNext();
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Ensure the specified item is within the client of the listbox.
//
////////////////////////////////////////////////////////////////////////
void RListBox::EnsureVisible(				// Returns nothing.
	RGuiItem* pgui,							// Item to ensure visibility of.
	Position	posPreference /*= Top*/)	// In:  Preferred vertical position.
	{
	short	sX	= pgui->m_sX;
	short	sY	= pgui->m_sY;

	// Take position of item to list contents level.
	RGuiItem*	pguiParent	= pgui->GetParent();
	while (pguiParent != NULL && pguiParent != &m_lcContents)
		{
		sX	+= pguiParent->m_sX;
		sY	+= pguiParent->m_sY;

		pguiParent	= pguiParent->GetParent();
		}

	// If we got to the list contents . . .
	if (pguiParent != NULL)
		{
		// Set new position for list contents such that item
		// is in upper left corner of client.
		short	sClientX, sClientY, sClientH;
		GetClient(&sClientX, &sClientY, NULL, &sClientH);

		switch (posPreference)
			{
			case Top:
				m_lcContents.Move(-sX + sClientX, -sY + sClientY);
				break;
			case Middle:
				m_lcContents.Move(-sX + sClientX, -sY + sClientY + (sClientH - pgui->m_im.m_sHeight) / 2);
				break;
			case Bottom:
				m_lcContents.Move(-sX + sClientX, -sY + sClientY + sClientH - pgui->m_im.m_sHeight);
				break;
			}

		// Update the scrollbars.
		UpdateScrollBars();
		}
	else
		{
		TRACE("EnsureVisible(): Specified gui is not a member of this "
			"listbox.\n");
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Set the selection to the specified GUI item.
//
////////////////////////////////////////////////////////////////////////
void RListBox::SetSel(				// Returns nothing.
	RGuiItem* pgui)					// Item to select or NULL for none.
	{
	// If new selection is different than current . . .
	if (pgui != m_pguiSel)
		{
		// Unselect item.
		SelectItem(m_pguiSel, FALSE);

		// Set new selection.
		m_pguiSel	= pgui;

		// Select item.
		SelectItem(m_pguiSel, TRUE);
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Update scrollbar visibility based on current listbox contents and
// their visibility.
//
////////////////////////////////////////////////////////////////////////
void RListBox::UpdateScrollBarVisibility(void)	// Returns nothing.
	{
	// Get client.
	short	sClientX, sClientY, sClientW, sClientH;
	GetClient(&sClientX, &sClientY, &sClientW, &sClientH);

	// If visibility is not prohibited . . .
	if (m_sbvVert != Hidden)
		{
		// If the vertical scrollbar should be visible . . .
		if (m_lcContents.m_im.m_sHeight > sClientH - m_sbHorz.m_im.m_sHeight || m_sbvVert == Shown)
			{
			m_sbVert.m_sVisible	= TRUE;
			}
		else
			{
			m_sbVert.m_sVisible	= FALSE;
			}
		}
	else
		{
		m_sbVert.m_sVisible	= FALSE;
		}

	// If visibility is not prohibited . . .
	if (m_sbvHorz != Hidden)
		{
		// If the horizontal scrollbar should be visible . . .
		if (m_lcContents.m_im.m_sWidth > sClientW - m_sbVert.m_im.m_sWidth || m_sbvHorz == Shown)
			{
			m_sbHorz.m_sVisible	= TRUE;
			}
		else
			{
			m_sbHorz.m_sVisible	= FALSE;
			}
		}
	else
		{
		m_sbHorz.m_sVisible	= FALSE;
		}

	// Activate scrollbars according to their visibility and current 
	// activation status of listbox.
	m_sbVert.SetActive(m_sbVert.m_sVisible && IsActivated());
	m_sbHorz.SetActive(m_sbHorz.m_sVisible && IsActivated());
	}

////////////////////////////////////////////////////////////////////////
//
// Resize encapsulators to fit the largest encapsulated item or string,
// reposition all items to appear in the correct order, resize scrollable
// area (m_lcContents) to fit all listbox items, and update scrollbar 
// ranges accordingly.
//
////////////////////////////////////////////////////////////////////////
void RListBox::AdjustContents(void)					// Returns nothing.
	{
	// Get client.
	short	sClientX, sClientY, sClientW, sClientH;
	GetClient(&sClientX, &sClientY, &sClientW, &sClientH);

	// Minimum is viewable area (when vertical scrollbar shown).
	m_sLargestWidth		= sClientW - m_sbVert.m_im.m_sWidth;

	long	lTotalHeight	= 0;	// Used for averaging heights.
	long	lTotalItems		= 0;	// Used for averaging heights.

	// Get largest width.
	RGuiItem*	pguiItem	= m_lcContents.m_listguiChildren.GetHead();
	while (pguiItem != NULL)
		{
		// If width is larger than largest . . .
		if (pguiItem->m_im.m_sWidth > m_sLargestWidth)
			{
			m_sLargestWidth	= pguiItem->m_im.m_sWidth;
			}

		// Get next item.
		pguiItem	= m_lcContents.m_listguiChildren.GetNext();
		}

	// Go through children sizing encapsulators to m_sLargestWidth and
	// repositioning items as we go.
	short	sY	= 0;
	pguiItem	= m_lcContents.m_listguiChildren.GetHead();
	while (pguiItem != NULL)
		{
		// Reposition.
		pguiItem->Move(pguiItem->m_sX, sY);

		// If this is an encapsulator . . .
		if (IsEncapsulator(pguiItem) != FALSE)
			{
			// If there's a change in width . . .
			if (pguiItem->m_im.m_sWidth != m_sLargestWidth)
				{
				// Recreate item . . .
				if (pguiItem->Create(
					pguiItem->m_sX,
					pguiItem->m_sY,
					m_sLargestWidth,
					pguiItem->m_im.m_sHeight,
					pguiItem->m_im.m_sDepth) == 0)
					{
					}
				else
					{
					TRACE("AdustContents(): pguiItem->Create() failed.\n");
					}
				}
			}

		// Adjust position for next item by this item's height and the
		// item spacing.
		sY	+= pguiItem->m_im.m_sHeight + ITEM_SPACING;

		// Keep total height so we can average it to determine scrollbar
		// increments.
		lTotalHeight	+= pguiItem->m_im.m_sHeight + ITEM_SPACING;
		lTotalItems++;

		// Get next item.
		pguiItem	= m_lcContents.m_listguiChildren.GetNext();
		}

	// New height and/or width for container . . .
	if (m_lcContents.Create(
		m_lcContents.m_sX,
		m_lcContents.m_sY,
		m_sLargestWidth,
		sY - ITEM_SPACING,
		m_lcContents.m_im.m_sDepth) == 0)
		{
		}
	else
		{
		TRACE("AdjustContents(): m_lcContents.Create() failed.\n");
		}

	// Update scrollbar visibility.
	UpdateScrollBarVisibility();

	// Set scroll ranges:

	// The scrollable range is the length of the frame minus the size of
	// the area that can be viewed.  This is subtracted from the client
	// position to so it is already mapped directly into 'this's 
	// coordinate system.

	// If the horizontal scrollbar is visible . . .
	short	sScrollBarH	= 0;
	if (m_sbHorz.m_sVisible != FALSE)
		{
		// Compensation:
		sScrollBarH	= m_sbHorz.m_im.m_sHeight;
		}

	// If viewable area is smaller than list area . . .
	if (m_lcContents.m_im.m_sHeight > sClientH - sScrollBarH)
		{
		m_sbVert.SetRange(
			-sClientY,
			(m_lcContents.m_im.m_sHeight - (sClientH - sScrollBarH)) - sClientY);
		}
	else
		{
		m_sbVert.SetRange(
			-sClientY,
			-sClientY);
		}
	
	// If the vertical scrollbar is visible . . .
	short	sScrollBarW	= 0;
	if (m_sbVert.m_sVisible != FALSE)
		{
		// Compensation:
		sScrollBarW	= m_sbVert.m_im.m_sWidth;
		}

	// If viewable area is smaller than list area . . .
	if (m_lcContents.m_im.m_sWidth > sClientW - sScrollBarW)
		{
		m_sbHorz.SetRange(
			-sClientX,
			(m_lcContents.m_im.m_sWidth - (sClientW - sScrollBarW)) - sClientX); 
		}
	else
		{
		m_sbHorz.SetRange(
			-sClientX,
			-sClientX);
		}

	// Calculate average height of item.
	long	lAvgHeight	= 0;
	if (lTotalItems > 0)
		{
		lAvgHeight	= (short)(lTotalHeight / lTotalItems);
		}
	
	// Update vertical scroll bar increments:
	
	// Clicking buttons should scroll one item at a time.
	m_sbVert.m_lButtonIncDec	= lAvgHeight;
	// Clicking tray should scroll visible items - 1 at a time.
	m_sbVert.m_lTrayIncDec		= (sClientH - sScrollBarH) - lAvgHeight;

	// Safety . . .
	if (m_sbVert.m_lTrayIncDec <= 0)
		{
		m_sbVert.m_lTrayIncDec	= lAvgHeight;
		}

	// Update scroll positions.
	UpdateScrollBars();
	}

////////////////////////////////////////////////////////////////////////
//
// Update scrollbars' positions.  This function is called by 
// AdjustContents() and EnsureVisible(), so there is no need to call it
// after calling one of those functions.
//
////////////////////////////////////////////////////////////////////////
void RListBox::UpdateScrollBars(void)
	{
	m_sbVert.SetPos(-m_lcContents.m_sY);
	m_sbHorz.SetPos(-m_lcContents.m_sX);
	}

////////////////////////////////////////////////////////////////////////
// Querries.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Get the first child item.
////////////////////////////////////////////////////////////////////////
RGuiItem* RListBox::GetFirst(void)	// Returns the first child item or
												// NULL.
	{
	// Find first encapsulator.
	RGuiItem*	pguiFirst	= m_lcContents.m_listguiChildren.GetHead();
	while (pguiFirst != NULL)
		{
		if (IsEncapsulator(pguiFirst) != FALSE)
			{
			break;
			}

		pguiFirst	= m_lcContents.m_listguiChildren.GetNext();
		}

	return pguiFirst;
	}

////////////////////////////////////////////////////////////////////////
// Get the next child item after the specified item.
////////////////////////////////////////////////////////////////////////
RGuiItem* RListBox::GetNext(		// Returns the next child item.
	RGuiItem*	pgui)					// In:  Child item that precedes the next.
	{
	RGuiItem*	pguiNext	= m_lcContents.m_listguiChildren.GetNext(pgui);
	while (pguiNext != NULL)
		{
		if (IsEncapsulator(pguiNext) != FALSE)
			{
			break;
			}

		pguiNext	= m_lcContents.m_listguiChildren.GetNext();
		}

	return pguiNext;
	}

////////////////////////////////////////////////////////////////////////
// Get the child item before the specified item.
////////////////////////////////////////////////////////////////////////
RGuiItem* RListBox::GetPrev(		// Returns the previous child item or NULL.
	RGuiItem*	pgui)					// In:  Child item that follows the prev.
	{
	RGuiItem*	pguiPrev	= m_lcContents.m_listguiChildren.GetPrev(pgui);
	while (pguiPrev != NULL)
		{
		if (IsEncapsulator(pguiPrev) != FALSE)
			{
			break;
			}

		pguiPrev	= m_lcContents.m_listguiChildren.GetPrev();
		}

	return pguiPrev;
	}

////////////////////////////////////////////////////////////////////////
// Internals.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Creates an item appropriate for the specified string and makes it
// a child of this listbox.
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RListBox::CreateStringItem(	// Returns new item on success;
													// NULL, otherwise.
	char* pszString)							// Text for new item.
	{
	RGuiItem*	pgui		= NULL;	// Assume nothing.
	short	sError	= 0;

	// We'll need a font and a print for this.
	ASSERT(m_pprint != NULL);
	ASSERT(m_pprint->GetFont() != NULL);

	// Allocate a new item . . .
	pgui	= CreateGuiItem(m_typeEncapsulator);
	if (pgui != NULL)
		{
		// Copy parms to new item.
		CopyParms(pgui);

		// Set callback.
		pgui->m_bcUser				= PressedCall;
		pgui->m_ulUserInstance	= (ULONG)this;

		// Get thickness of border.
		short sBorder	= pgui->GetTopLeftBorderThickness()
			+ pgui->GetBottomRightBorderThickness();

		// Get the width of the desired string.
		short	sWidth	= m_pprint->GetWidth(pszString);
		short	sHeight	= m_sFontCellHeight;

		// Adjust by border thickness.
		sWidth	+= sBorder;
		sHeight	+= sBorder;

		// Set text for item before Create() (Create() calls Compose()).
		pgui->SetText(pszString);

		// Create item . . .
		if (pgui->Create(0, 0, sWidth, sHeight, m_im.m_sDepth) == 0)
			{
			// Make child of list area.
			pgui->SetParent(&m_lcContents);
			// Remember to delete this item.
			MakeEncapsulator(pgui);
			// Item position is not set until the next AdjustContents() call.
			}
		else
			{
			TRACE("CreateStringItem(): ptxt->Create() failed.\n");
			sError	= 2;
			}

		// If any errors occurred after allocation . . .
		if (sError != 0)
			{
			delete pgui;
			pgui	= NULL;
			}
		}
	else
		{
		TRACE("CreateStringItem(): Failed to allocate encapsulator of type %s.\n",
			ms_apszTypes[m_typeEncapsulator]);
		sError	= 1;
		}

	return pgui;
	}

////////////////////////////////////////////////////////////////////////
//
// Creates an encapsulator object for the specified GUI.
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RListBox::CreateEncapsulator(	// Returns new item on success; NULL,
														// otherwise.
	RGuiItem*	pgui)								// Item to encapsulate.
	{
	RGuiItem*	pguiRes		= NULL;	// Assume nothing.
	short	sError	= 0;

	// Allocate a new item . . .
	pguiRes	= new RGuiItem;
	if (pguiRes != NULL)
		{
		// Copy parms to new item.
		CopyParms(pguiRes);

		// Set callback.
		pguiRes->m_bcUser				= PressedCall;
		pguiRes->m_ulUserInstance	= (ULONG)this;

		// Get thickness of border.
		short sBorder	= pguiRes->GetTopLeftBorderThickness()
			+ pguiRes->GetBottomRightBorderThickness();

		// Get the width and height of the encapsulated item.
		short	sWidth	= pgui->m_im.m_sWidth;
		short	sHeight	= pgui->m_im.m_sHeight;

		// Adjust by border thickness.
		sWidth	+= sBorder;
		sHeight	+= sBorder;

		// Create item . . .
		if (pguiRes->Create(0, 0, sWidth, sHeight, m_im.m_sDepth) == 0)
			{
			// Make item a child of encapsulator.
			pgui->SetParent(pguiRes);
			// Mark item as an encapsulator.
			MakeEncapsulator(pguiRes);
			// Item position is not set until the next AdjustContents() call.
			}
		else
			{
			TRACE("CreateEncapsulator(): pguiRes->Create() failed.\n");
			sError	= 2;
			}

		// If any errors occurred after allocation . . .
		if (sError != 0)
			{
			delete pguiRes;
			pguiRes	= NULL;
			}
		}
	else
		{
		TRACE("CreateEncapsulator(): Failed to allocate RTxt.\n");
		sError	= 1;
		}

	return pguiRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Place item at specified location in list of container's child items.
// Under new pretenses, this should not fail.
//
//////////////////////////////////////////////////////////////////////////////
void RListBox::AddAfter(		// Returns nothing.
	RGuiItem*	pgui,				// Item to add.
	RGuiItem*	pguiAfter)		// Item to add after or NULL to add at
										// end.
	{
	// Reposition the new item.  Under new pretenses, this SHOULD not fail.
	m_lcContents.m_listguiChildren.Remove(pgui);

	// If none specified . . .
	if (pguiAfter == NULL)
		{
		// Insert new item.  Under new pretenses, this SHOULD not fail.
		m_lcContents.m_listguiChildren.AddTail(pgui);
		}
	else
		{
		// Insert new item.  Under new pretenses, this SHOULD not fail.
		m_lcContents.m_listguiChildren.InsertAfter(pguiAfter, pgui);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Place item at specified location in list of container's child items.
// Under new pretenses, this should not fail.
//
//////////////////////////////////////////////////////////////////////////////
void RListBox::InsertBefore(	// Returns nothing.
	RGuiItem*	pgui,				// Item to insert.
	RGuiItem*	pguiBefore)		// Item to insert before or NULL to insert
										// at beginning.
	{
	// Reposition the new item.  Under new pretenses, this SHOULD not fail.
	m_lcContents.m_listguiChildren.Remove(pgui);

	// If none specified . . .
	if (pguiBefore == NULL)
		{
		// Insert new item.  Under new pretenses, this SHOULD not fail.
		m_lcContents.m_listguiChildren.InsertHead(pgui);
		}
	else
		{
		// Insert new item.  Under new pretenses, this SHOULD not fail.
		m_lcContents.m_listguiChildren.InsertBefore(pguiBefore, pgui);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// [Un]Select the specified item.
//
//////////////////////////////////////////////////////////////////////////////
void RListBox::SelectItem(		// Returns nothing.
	RGuiItem*	pguiSel,			// Item to [un]select.
	short			sSelect)			// If TRUE, item is selected; if FALSE,
										// item is unselected.
	{
	// If there was previously a selection . . .
	if (pguiSel != NULL)
		{
		// If this is an encapsulation . . .
		if (IsEncapsulator(pguiSel) != FALSE)
			{
			// Toggle border.
			pguiSel->m_sInvertedBorder	= sSelect;
			// Recompose item with new border effect.
			pguiSel->Compose();
			}
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Load item's children from the specified file.
// (virtual override).
// (protected).
//
////////////////////////////////////////////////////////////////////////
short RListBox::LoadChildren(	// Returns 0 on success.
	RFile*	pfile)				// File to load from.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pfile->IsOpen() != FALSE);

	short	sNum;
	// Read number of children.
	pfile->Read(&sNum);

	// The first three are our scrollbars and frame.
	ASSERT(sNum >= 3);

	// Load directly into these special children.
	if (m_sbVert.Load(pfile) == 0)
		{
		if (m_sbHorz.Load(pfile) == 0)
			{
			// m_lcContents needs to know its parent during Load() (specifically
			// LoadChildren() needs to know).
			m_lcContents.SetParent(this);

			if (m_lcContents.Load(pfile) == 0)
				{
				// Subtract these three children from total.
				sNum	-= 3;
				}
			else
				{
				TRACE("LoadChildren(): m_sbVert.Load() failed.\n");
				sRes	= -3;
				}
			}
		else
			{
			TRACE("LoadChildren(): m_sbHorz.Load() failed.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("LoadChildren(): m_lcContents.Load() failed.\n");
		sRes	= -1;
		}

	// Instantiate rest of children.
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
// Save item's children to the specified file.
// (virtual override).
// (protected).
//
////////////////////////////////////////////////////////////////////////
short RListBox::SaveChildren(	// Returns 0 on success.
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

	// These should definitely be children of 'this' item.
	ASSERT(m_sbVert.GetParent()		== this);
	ASSERT(m_sbHorz.GetParent()		== this);
	ASSERT(m_lcContents.GetParent()	== this);

	// Always write our 3 special children (scrollbars & frame) first so we know
	// where to get them on load.
	if (m_sbVert.Save(pfile) == 0)
		{
		if (m_sbHorz.Save(pfile) == 0)
			{
			if (m_lcContents.Save(pfile) == 0)
				{
				// Subtract these three children from total.
				// Currently this number is not used during save,
				// but just in case it ever is.
				sNum	-= 3;
				}
			else
				{
				TRACE("SaveChildren(): m_lcContents.Save() failed.\n");
				sRes	= -3;
				}
			}
		else
			{
			TRACE("SaveChildren(): m_sbHorz.Save() failed.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("SaveChildren(): m_sbVert.Save() failed.\n");
		sRes	= -1;
		}

	// Save children.  Note that we go through the children in reverse
	// order so they, on load, get added back to their parent in the
	// order they were originally added to this parent.
	pgui	= m_listguiChildren.GetTail();
	while (pgui != NULL && sRes == 0 && pfile->Error() == FALSE)
		{
		// Don't write these 3 again . . .
		if (pgui != &m_sbVert && pgui != &m_sbHorz && pgui != &m_lcContents)
			{
			// Save child.
			sRes	= pgui->Save(pfile);
			}

		pgui	= m_listguiChildren.GetPrev();
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Read item's members from file.
// (virtual/protected (overriden here)).
//
////////////////////////////////////////////////////////////////////////
short RListBox::ReadMembers(	// Returns 0 on success.
	RFile*	pfile,					// File to read from.
	U32		u32Version)				// File format version to use.
	{
	short	sRes	= 0;	// Assume success.

	// Invoke base class to read base members.
	sRes	= RGuiItem::ReadMembers(pfile, u32Version);

	// If okay so far . . .
	if (sRes == 0)
		{
		ASSERT(pfile != NULL);
		ASSERT(pfile->IsOpen() != FALSE);
		
		U32	u32Temp;
		
		// Switch on version.
		switch (u32Version)
			{
			default:
			// Insert additional version numbers here!
			// case 4:	// Version 4 stuff.
			// case 3:	// Version 3 stuff.
			case 2:	// Version 2 added encapsulator type.
				pfile->Read(&u32Temp);
				m_typeEncapsulator	= (Type)u32Temp;

			case 1:
				// Read this class's members.
				pfile->Read(&u32Temp);
				m_sbvVert	= (ScrollBarVisibility)u32Temp;

				pfile->Read(&u32Temp);
				m_sbvHorz	= (ScrollBarVisibility)u32Temp;

			case 0:	// In version 0, only base class RGuiItem members were stored.
				// If successful . . .
				if (pfile->Error() == FALSE)
					{
					// Success.
					}
				else
					{
					TRACE("ReadMembers(): Error reading RListBox members.\n");
					sRes	= -1;
					}
				break;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Write item's members to file.
// (virtual/protected (overriden here)).
//
////////////////////////////////////////////////////////////////////////
short RListBox::WriteMembers(	// Returns 0 on success.
	RFile*	pfile)					// File to write to.
	{
	short	sRes	= 0;	// Assume success.

	// Invoke base class to read base members.
	sRes	= RGuiItem::WriteMembers(pfile);

	// If okay so far . . .
	if (sRes == 0)
		{
		ASSERT(pfile != NULL);
		ASSERT(pfile->IsOpen() != FALSE);
		

		// Write this class's members.
		////////////// Version 2 ////////////////////
		pfile->Write((U32)m_typeEncapsulator);	
		////////////// Version 1 ////////////////////
		pfile->Write((U32)m_sbvVert);
		pfile->Write((U32)m_sbvHorz);
		////////////// Version 0 ////////////////////

		// If successful . . .
		if (pfile->Error() == FALSE)
			{
			// Success.
			}
		else
			{
			TRACE("WriteMembers(): Error writing RListBox members.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
