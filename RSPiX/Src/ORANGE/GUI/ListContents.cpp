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
// ListContents.CPP
// 
// History:
//		01/21/97 JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// This is a very simple class designed to be used as a container specifically
// for use with an RListBox.
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
#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/ListBox.h"
#else
	#include "ListBox.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Internal.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Save item's children to the specified file.
// (protected/virtual (overridden here)).
//
//////////////////////////////////////////////////////////////////////////////
short RListContents::SaveChildren(	// Returns 0 on success.
	RFile*	pfile)						// File to save to.
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

	// Save children in reverse order.
	pgui	= m_listguiChildren.GetTail();
	while (pgui != NULL && sRes == 0 && pfile->Error() == FALSE)
		{
		// Before each item is a value indicating whether the item
		// is an encapsulator.
		pfile->Write((short)pgui->IsProp(ENCAPSULATOR_PROP_KEY) );

		// Save child.
		sRes	= pgui->Save(pfile);

		pgui	= m_listguiChildren.GetPrev();
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Load item's children from the specified file.
// (protected/virtual (overridden here)).
//
//////////////////////////////////////////////////////////////////////////////
short RListContents::LoadChildren(	// Returns 0 on success.
	RFile*	pfile)						// File to load from.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pfile->IsOpen() != FALSE);
	// Need to know parent.
	ASSERT(GetParent() != NULL);

	short	sNum;
	// Read number of children.
	pfile->Read(&sNum);

	// Instantiate children.
	RGuiItem* pgui;
	short	sCurChild;
	short	sEncapsulator;
	for (	sCurChild	= 0; 
			sCurChild < sNum && sRes == 0 && pfile->Error() == FALSE; 
			sCurChild++)
		{
		// Before each item is a value indicating whether the item
		// is an encapsulator.
		pfile->Read(&sEncapsulator);

		pgui	= LoadInstantiate(pfile);
		if (pgui != NULL)
			{
			pgui->SetParent(this);
			// If the item is an encapsulator . . .
			if (sEncapsulator != FALSE)
				{
				// Mark item as an encapsulator.
				pgui->SetProp(ENCAPSULATOR_PROP_KEY, TRUE);
				// Set callback.
				pgui->m_bcUser				= RListBox::PressedCall;
				// Set instance to parent listbox.
				pgui->m_ulUserInstance	= (ULONG)GetParent();
				// If pushed in . . .
				if (pgui->m_sInvertedBorder != FALSE)
					{
					// Select item.
					((RListBox*)GetParent())->SetSel(pgui);
					}
				}
			}
		else
			{
			TRACE("LoadChildren(): LoadInstantiate() failed.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
