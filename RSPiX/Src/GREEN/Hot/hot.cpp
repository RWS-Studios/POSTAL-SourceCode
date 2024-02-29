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
//	Hot.cpp
// 
// History:
//		06/13/96 JMI	Started.  Tried to maintain compatability with resonably
//							useful functionalities of the Win32 specific RHot.
//
//		09/24/96	JMI	Changed all BLU_MB?_* macros to RSP_MB?_* macros.
//
//		10/30/96	JMI	Changed:
//							Old label		New Label
//							=========		=========
//							CHot				RHot
//							HOTCALL			REventCall
//							HOTCALL2			REventPosCall
//							CList				RList
//
//		01/01/97	JMI	Added prioritized hotboxes.  Defaults to non-prioritized.
//							Calling SetPriority(sPriority > RHOT_NO_PRIORITY) causes use of 
//							prioritization for this RHot.
//
//		01/04/97	JMI	Added ability to have parent-child RHot relationships.
//							This should simplify things that use this class that
//							utilize such relationships.
//
//		01/06/97	JMI	Added m_listChildren to keep track of children.  I was
//							afraid that there needed to be a way to let child hots
//							when the parent was destroyed.  Now, when a parent is
//							destroyed, it goes through all its children doing a 
//							photChild->SetParent(NULL).
//
//		01/13/97	JMI	Added Do() that takes an event.  Now the old Do() calls
//							this when it finds an event.
//
//		01/15/97	JMI	Do(short) was calling Do(short, short, short) with the
//							event as the first (instead of the last) parameter.
//
//		01/21/97	JMI	The main Do() now returns the priority of the RHot that
//							got the event.
//
//		01/22/97	JMI	Changed DoEvent() to DoChildren().  Reversed order of
//							pos vs. event args.
//
//		01/23/97	JMI	Got rid of the whole implied root with the static lists.
//							Now each hotbox can service its children via a Do() call.
//							More of a typical child-parent relationship.  Note that
//							this got rid of the Do(short sUseQueue) static that
//							dequeued or polled events, but it was getting stale any-
//							ways.
//
//		01/26/97	JMI	Changed callbacks' first parms to RHot* instead of ULONG.
//
//		03/19/97	JMI	Added InputEventCall and made Do(pie) the main RHot
//							interface.
//							Also, added new constructor overload to handle new call-
//							back type.
//
//		03/19/97	JMI	Now evaluates capture hotboxes first and stores the
//							priority of the highest priority box which can block
//							processing of non-capture boxes.
//							Also, captured hotboxes were being passed the wrong RHot
//							pointer.  Fixed.
//
//		03/28/97	JMI	Priority of capture hotboxes was not being stored 
//							properly.  Fixed.
//
//////////////////////////////////////////////////////////////////////////////
//
// Offers simple hotbox services based on either the mouse queue or polling.
//
// Intricasies(sp?) regarding priorities: 
//	-	Priority of RHOT_NO_PRIORITY indicates non-prioritized hotbox.
//		More than one non-prioritized hotbox can receive a callback
//		in response to a single mouse event.
//	-	Only one prioritized hotbox can receive a callback in
//		response to a single mouse event.
//	-	Any number of non-prioritized and one prioritized hotbox
//		can receive a callback in response to a single mouse event.
//
//////////////////////////////////////////////////////////////////////////////

// Blue //////////////////////////////////////////////////////////////////////
#include "Blue.h"

// Green /////////////////////////////////////////////////////////////////////
// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Hot/hot.h"
#else
	#include "hot.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Instanstiate/Initialize class statics.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific non-member functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Class functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RHot::RHot()
	{
	// Reset all members.
	Init();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Constructura Especial that sets some intial values.
//
//////////////////////////////////////////////////////////////////////////////
RHot::RHot(
	short sX,										// X position of new hotbox.
	short sY,										// Y position of new hotbox.
	short sW,										// Width of new hotbox.
	short sH,										// Height of new hotbox.
	REventCall fnEventCall /*= NULL*/,		// Callback on mouse event.
	short	sActive /*= FALSE*/,					// Initially active, if TRUE.
	ULONG	ulUser /*= 0*/,						// User value.
	short sPriority /*= RHOT_NO_PRIORITY*/)// Priority.  Default == non-prioritized.
	{
	// Reset all members.
	Init();
	
	m_sX	= sX;
	m_sY	= sY;
	m_sW	= sW;
	m_sH	= sH;

	m_ulUser		= ulUser;

	m_ecUser		= fnEventCall;

	SetPriority(sPriority);

	// Set activation status.
	SetActive(sActive);
	}
		
//////////////////////////////////////////////////////////////////////////////
//
// Constructura Especial that sets some intial values.
//
//////////////////////////////////////////////////////////////////////////////
RHot::RHot(
	short sX,										// X position of new hotbox.
	short sY,										// Y position of new hotbox.
	short sW,										// Width of new hotbox.
	short sH,										// Height of new hotbox.
	REventPosCall fnEventPosCall,				// Callback on mouse event.
	short	sActive /*= FALSE*/,					// Initially active, if TRUE.
	ULONG	ulUser /*= 0*/,						// User value.
	short sPriority /*= RHOT_NO_PRIORITY*/)// Priority.  Default == non-prioritized.
	{
	// Reset all members.
	Init();
	
	m_sX	= sX;
	m_sY	= sY;
	m_sW	= sW;
	m_sH	= sH;

	m_epcUser	= fnEventPosCall;

	m_ulUser		= ulUser;

	SetPriority(sPriority);

	// Set activation status.
	SetActive(sActive);
	}
		
//////////////////////////////////////////////////////////////////////////////
//
// Constructura Especial that sets some intial values.
//
//////////////////////////////////////////////////////////////////////////////
RHot::RHot(
	short sX,										// X position of new hotbox.
	short sY,										// Y position of new hotbox.
	short sW,										// Width of new hotbox.
	short sH,										// Height of new hotbox.
	InputEventCall fnInputEventCall,			// Callback on mouse event.
	short	sActive /*= FALSE*/,					// Initially active, if TRUE.
	ULONG	ulUser /*= 0*/,						// User value.
	short sPriority /*= RHOT_NO_PRIORITY*/)// Priority.  Default == non-prioritized.
	{
	// Reset all members.
	Init();
	
	m_sX	= sX;
	m_sY	= sY;
	m_sW	= sW;
	m_sH	= sH;

	m_iecUser	= fnInputEventCall;

	m_ulUser		= ulUser;

	SetPriority(sPriority);

	// Set activation status.
	SetActive(sActive);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RHot::~RHot()
	{
	// Deactivate.
	SetActive(FALSE);
	// Decapture.
	SetCapture(FALSE);

	// Release all children.
	RHot*	phot	= m_listChildren.GetHead();
	while (phot != NULL)
		{
		phot->SetParent(NULL);

		phot	= m_listChildren.GetNext();
		}

	// Release parent.
	SetParent(NULL);
	}

//////////////////////////////////////////////////////////////////////////////
// Manipulations.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Activates/Deactivates hotbox.  When active, the hotbox
// calls the callback when mouse events occur.
//
//////////////////////////////////////////////////////////////////////////////
void RHot::SetActive(	// Returns nothing.
	short sActive)			// TRUE to activate, FALSE otherwise.
	{
	if (m_sActive != sActive)
		{
		if (sActive == TRUE)
			{
			// If this item has a parent . . .
			RHot*	photParent	= GetParent();
			if (photParent != NULL)
				{
				// Add us into its list.
				if (photParent->m_slistActiveChildren.Insert(this, &m_sPriority) == 0)
					{
					// Success.
					m_sActive	= TRUE;
					}
				else
					{
					TRACE("SetActive(): Failed to insert into list.  Delete me; I'm useless.\n");
					}
				}
			else
				{
				m_sActive	= TRUE;
				}
			}
		else
			{
			// If this item has a parent . . .
			RHot*	photParent	= GetParent();
			if (photParent != NULL)
				{
				// Remove us from list.
				if (photParent->m_slistActiveChildren.Remove(this) == 0)
					{
					// Success.
					m_sActive	= FALSE;
					}
				else
					{
					TRACE("SetActive(): Failed to remove from list.\n");
					}
				}
			else
				{
				m_sActive	= FALSE;
				}
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets the priority of this hotbox.
// If hotbox is already active, it is repositioned in the prioritized 
// list so that the new priorty is recognized.
// See CPP comment header in regards to specifics of this value.
//
//////////////////////////////////////////////////////////////////////////////
void RHot::SetPriority(	// Returns 0 on success.
	short sPriority)			// New priority for hotbox.  Lower value
									// equals higher priority.
									// RHOT_NO_PRIORITY(default) indicates non-prioritized.
	{
	// Change priority.
	m_sPriority	= sPriority;

	// If this hotbox is active . . .
	if (m_sActive != FALSE)
		{
		// If this item has a parent . . .
		RHot*	photParent	= GetParent();
		if (photParent != NULL)
			{
			// Reposition with new priority.
			if (photParent->m_slistActiveChildren.Reposition(this) == 0)
				{
				// Success.
				}
			else
				{
				TRACE("SetPriority(): photParent->m_listChildren.Reposition() failed.\n");
				}
			}
		}
	}


//////////////////////////////////////////////////////////////////////////////
//
// Sets this hotbox's parent to the specified hotbox.
// This has the effect of having the hotbox scanned relative to the
// parent and only within the area of the parent.
//
//////////////////////////////////////////////////////////////////////////////
void RHot::SetParent(	// Returns nothing.
	RHot* photParent)		// Hotbox to be parent of this hotbox or NULL
								// for none.
	{
	ASSERT(photParent != this);

	// Store activation status.
	short	sActive	= IsActive();
	// Store capture status.
	short	sCapture	= IsCapturing();

	// If active . . .
	if (sActive != FALSE)
		{
		// Deactivate.
		SetActive(FALSE);
		}

	// If capturing . . .
	if (sCapture != FALSE)
		{
		// Decapture.
		SetCapture(FALSE);
		}

	// If there's an existing parent . . .
	if (m_photParent != NULL)
		{
		// Remove from its list of children.
		m_photParent->m_listChildren.Remove(this);
		}

	// Set new parent.
	m_photParent	= photParent;

	// If there's a new parent . . .
	if (m_photParent != NULL)
		{
		// Remove from its list of children.
		m_photParent->m_listChildren.AddTail(this);
		}

	// If active . . .
	if (sActive != FALSE)
		{
		// Activate.
		SetActive(TRUE);
		}

	// If capturing . . .
	if (sCapture != FALSE)
		{
		// Capture.
		SetCapture(TRUE);
		}

	// We should be back in original shape.
	ASSERT(sActive == IsActive() );
	ASSERT(sCapture == IsCapturing() );
	}

//////////////////////////////////////////////////////////////////////////////
//
// Activates/Deactivates capturing for this hotbox.
// When capturing is active, this hotbox always receives
// events.  Sort of a cursor event capture mode.
//
//////////////////////////////////////////////////////////////////////////////
void RHot::SetCapture(	// Returns nothing.
	short sActive)			// TRUE to activate, FALSE otherwise.
	{
	if (m_sCapture != sActive)
		{
		if (sActive == TRUE)
			{
			// Add us into list . . .
			if (GetCaptureList()->Insert(this) == 0)
				{
				// Success.
				m_sCapture	= TRUE;
				}
			else
				{
				TRACE("SetCapture(): Failed to insert into list.  Delete me; I'm useless.\n");
				}
			}
		else
			{
			// Remove us from list . . .
			if (GetCaptureList()->Remove(this) == 0)
				{
				// Success.
				m_sCapture	= FALSE;
				}
			else
				{
				TRACE("SetCapture(): Failed to remove from list.\n");
				}
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Processes a single event for all child hotboxes.
// Non mouse events are ignored.
// It is now up to the callback to check whether the input event
// is used and decide whether to utilize the event.
// Recurses on children.
// This is called by the other Do().
//
//////////////////////////////////////////////////////////////////////////////
short	RHot::Do(			// Returns priority of item called back or 
								// RHOT_NO_PRIORITY.
	RInputEvent*	pie)	// In:  Most recent user input event.
								// Out: Depends on callbacks.  Generally,
								// pie->sUsed = TRUE, if used.
	{
	short	sPriorityCalled	= RHOT_NO_PRIORITY;	// Assume no callback.

	// Make sure we're dealing with the correct event type . . .
	if (pie->type == RInputEvent::Mouse)
		{
		// If this item is top-level . . .
		if (m_photParent == NULL)
			{
			// Process capture items.
			// Notify all hotboxes that are capturing.
			// Note that capture items have no priority.
			short sChildPosX;
			short	sChildPosY;
			RHot* phot	= m_listCapturing.GetHead();
			while (phot != NULL)
				{
				// Notify user via callback(s).
				if (phot->m_ecUser != NULL)
					{
					(*phot->m_ecUser)(phot, pie->sEvent);
					}

				if (phot->m_epcUser != NULL)
					{
					sChildPosX	= pie->sPosX;
					sChildPosY	= pie->sPosY;

					// Get the child position equivalent.
					phot->GetChildPos(&sChildPosX, &sChildPosY);

					(*phot->m_epcUser)(phot, pie->sEvent, sChildPosX, sChildPosY);
					}

				if (phot->m_iecUser != NULL)
					{
					// Get the child position equivalent.
					phot->GetChildPos(&pie->sPosX, &pie->sPosY);

					(*phot->m_iecUser)(phot, pie);

					// Convert back to parent coords.
					phot->GetTopPos(&pie->sPosX, &pie->sPosY);
					}

				// Store priority.
				if (sPriorityCalled == RHOT_NO_PRIORITY)
					{
					sPriorityCalled	= phot->m_sPriority;
					}
				else
					{
					sPriorityCalled	= MIN(sPriorityCalled, phot->m_sPriority);
					}

				phot	= m_listCapturing.GetNext();
				}
			}

		// Is the event inside this region . . .
		if (	pie->sPosX >= m_sX 
			&&	pie->sPosY >= m_sY
			&&	pie->sPosX < m_sX + m_sW
			&& pie->sPosY < m_sY + m_sH)
			{
			// Get first child.
			RHot*	photChild			= m_slistActiveChildren.GetHead();
			// While there are more children and no prioritized callback has occurred.
			while (photChild != NULL)
				{
				ASSERT(photChild != this);

				// If this item is non-prioritized or no prioritized callback
				// has yet occurred . . .
				if (sPriorityCalled == RHOT_NO_PRIORITY || photChild->m_sPriority == RHOT_NO_PRIORITY)
					{
					// Convert to child coords.
					pie->sPosX	-= m_sX;
					pie->sPosY	-= m_sY;

					// Process event in child using our coordinate system.
					sPriorityCalled	= photChild->Do(pie);

					// Convert back to parent coords.
					pie->sPosX	+= m_sX;
					pie->sPosY	+= m_sY;
					}

				// Get next child.
				photChild	= m_slistActiveChildren.GetNext();
				}

			// If no prioritized item yet called or this is a non-prioritized item 
			// and this item is not capturing . . .
			if ((sPriorityCalled == RHOT_NO_PRIORITY || m_sPriority == RHOT_NO_PRIORITY)
				&& m_sCapture == FALSE)
				{
				// Notify user via callback(s).
				if (m_ecUser != NULL)
					{
					(*m_ecUser)(this, pie->sEvent);

					// Store priority.
					sPriorityCalled	= m_sPriority;
					}

				if (m_epcUser != NULL)
					{
					(*m_epcUser)(this, pie->sEvent, pie->sPosX, pie->sPosY);

					// Store priority.
					sPriorityCalled	= m_sPriority;
					}

				if (m_iecUser != NULL)
					{
					(*m_iecUser)(this, pie);

					// Store priority.
					sPriorityCalled	= m_sPriority;
					}
				}
			}
		}

	return sPriorityCalled;
	}

//////////////////////////////////////////////////////////////////////////////
// Querries.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Get the child position equivalent coordinates.
//
//////////////////////////////////////////////////////////////////////////////
void RHot::GetChildPos(	// Returns nothing.
	short* psX,				// In:  Top-level position.
								// Out: Child position.
	short* psY)				// In:  Top-level position.
								// Out: Child position.
	{
	RHot*	photParent	= m_photParent;
	while (photParent != NULL)
		{
		// Move through parent's coordinate system.
		*psX	-= photParent->m_sX;
		*psY	-= photParent->m_sY;

		// Get next parent.
		photParent	= photParent->m_photParent;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get the top position equivalent coordinates.
//
//////////////////////////////////////////////////////////////////////////////
void RHot::GetTopPos(	// Returns nothing.
	short* psX,				// In:  Child position.
								// Out: Top-level position.
	short* psY)				// In:  Child position.
								// Out: Top-level position.    
	{
	RHot*	photParent	= m_photParent;
	while (photParent != NULL)
		{
		// Move through parent's coordinate system.
		*psX	+= photParent->m_sX;
		*psY	+= photParent->m_sY;

		// Get next parent.
		photParent	= photParent->m_photParent;
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Internal.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Inits all members of this RHot.
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
void RHot::Init(void)
	{
	// Initially not active.
	m_sActive	= FALSE;
	// Clear callbacks.
	m_ecUser		= NULL;
	m_epcUser	= NULL;
	m_iecUser	= NULL;
	// Clear user value.
	m_ulUser		= 0L;
	// Initialize positions.
	m_sX	= m_sY	= m_sW	= m_sH	= 0;
	// Non-prioritized.
	m_sPriority		= RHOT_NO_PRIORITY;
	// Global; no parent.
	m_photParent	= NULL;
	// Not capturing events.
	m_sCapture		= FALSE;
	}
	
//////////////////////////////////////////////////////////////////////////////
//
// Gets the list appropriate for this hotbox.
//
//////////////////////////////////////////////////////////////////////////////
RHot::ListHots* RHot::GetCaptureList(void)	// Returns Capture list 
															// appropriate for this RHot.
															// Cannot fail.
	{
	// Go to highest level.
	RHot*	phot			= this;
	RHot*	photParent	= GetParent();
	while (photParent != NULL)
		{
		phot			= photParent;
		photParent	= phot->GetParent();
		}

	// Use this RHot's capture list.
	return &(phot->m_listCapturing);
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
