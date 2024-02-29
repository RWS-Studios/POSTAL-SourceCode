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
//	Hot.h
// 
// History:
//		01/18/97 JMI	Started tracking history of this file.
//							Added version of Do() that takes an RInputEvent*.
//
//		01/21/97	JMI	The main Do() now returns the priority of the RHot that
//							got the event.  Now, Do(RInputEvent* pie) can set
//							pie->sUsed = TRUE discriminantly.
//
//		01/22/97	JMI	Added DoChildren() to process an event for this hotbox's
//							children with two overloads (sPosX,sPosY,sEvent & pie).
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
//////////////////////////////////////////////////////////////////////////////
//
// See CPP for description.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef HOT_H
#define HOT_H
// Orange /////////////////////////////////////////////////////////////////////
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/InputEvent/InputEvent.h"
	#include "ORANGE/CDT/slist.h"
	#include "ORANGE/CDT/List.h"
#else
	#include "slist.h"
	#include "List.h"
	#include "InputEvent.h"
#endif // PATHS_IN_INCLUDES

///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////

// This, when used in place of a priority, indicates that the item has no
// priority (i.e., it is to be considered non-prioritized).
#define RHOT_NO_PRIORITY	( (short)0x8000)

///////////////////////////////////////////////////////////////////////////////
// Types.
///////////////////////////////////////////////////////////////////////////////
class RHot
	{
	public:	// Typedefs.
		typedef void (*REventCall)(	// Returns nothing.
						RHot*	phot,			// Ptr to the RHot that generated the
												// callback.
						short sEvent);		// Event that occurred.
												// Uses blue.h macros (RSP_MB?_*).

		typedef void (*REventPosCall)(	// Returns nothing.
						RHot*	phot,				// Ptr to the RHot that generated the
													// callback.
						short sEvent,			// Event that occurred.
													// Uses blue.h macros (RSP_MB?_*).
						short sPosX,			// Mouse x position at button event
													// relative to hotbox's upper, left
													// corner.
						short sPosY);			// Mouse y position at button event
													// relative to hotbox's upper, left
													// corner.

		typedef void (*InputEventCall)(	// Returns nothing.
						RHot* phot,				// In:  Ptr to RHot that generated the
													// callback.
						RInputEvent*	pie);	// In:  Ptr to input event that generated
													// the callback.  Note that the coordinates
													// of this event are relative to hotbox's
													// upper, left corner.
													// Out: Set members as you see fit.  Note
													// that changes to pie->sPosX,sPosY should
													// be made relative to this RHot's upper,
													// left corner.  This can affect further 
													// RHot callbacks and will certainly affect 
													// the RInputEvent passed to RHot::Do().

		typedef RSList <RHot, short>	SListHots;	// Sorted list of RHots.
		typedef RList <RHot>				ListHots;	// List of RHots.

	public:
		// Default constructor.
		RHot();

		// Constructura Especial that sets some intial values.
		RHot(	
			short sX,									// X position of new hotbox.
			short sY,									// Y position of new hotbox.
			short sW,									// Width of new hotbox.
			short sH,									// Height of new hotbox.
			REventCall fnEventCall = NULL,		// Callback on mouse event.
			short	sActive	= FALSE,					// Initially active, if TRUE.
			ULONG	ulUser	= 0,						// User value.
			short sPriority = RHOT_NO_PRIORITY);// Priority.  Default == non-prioritized.

		// Constructura Especial el Segundario or something that sets some
		// intial values.
		RHot(	
			short sX,									// X position of new hotbox.
			short sY,									// Y position of new hotbox.
			short sW,									// Width of new hotbox.
			short sH,									// Height of new hotbox.
			REventPosCall fnEventPosCall,			// Callback on mouse event.
			short	sActive	= FALSE,					// Initially active, if TRUE.
			ULONG	ulUser	= 0,						// User value.
			short sPriority = RHOT_NO_PRIORITY);// Priority.  Default == non-prioritized.

		// Constructura Especial el Tres or something that sets some
		// intial values.
		RHot(	
			short sX,									// X position of new hotbox.
			short sY,									// Y position of new hotbox.
			short sW,									// Width of new hotbox.
			short sH,									// Height of new hotbox.
			InputEventCall fnInputEventCall,		// Callback on mouse event.
			short	sActive	= FALSE,					// Initially active, if TRUE.
			ULONG	ulUser	= 0,						// User value.
			short sPriority = RHOT_NO_PRIORITY);// Priority.  Default == non-prioritized.

		// Destructor.
		~RHot();

	public:	// Manipulations.
		// Activates/Deactivates hotbox.  When active, the hotbox
		// calls the callback(s) when mouse events occur within its
		// area.
		void SetActive(		// Returns nothing.
			short sActive);	// TRUE to activate, FALSE otherwise.

		// Sets the priority of this hotbox.
		// If hotbox is already active, it is repositioned in the prioritized 
		// list so that the new priorty is recognized.
		// See CPP comment header in regards to specifics of this value.
		void SetPriority(		// Returns nothing.
			short sPriority);	// New priority for hotbox.  Lower value
									// equals higher priority.
									// RHOT_NO_PRIORITY(default) indicates non-prioritized.

		// Sets this hotbox's parent to the specified hotbox.
		// This has the effect of having the hotbox scanned relative to the
		// parent and only within the area of the parent.
		void SetParent(			// Returns nothing.
			RHot* photParent);	// Hotbox to be parent of this hotbox or NULL
										// for none.

		// Activates/Deactivates capturing for this hotbox.
		// When capturing is active, this hotbox always receives
		// events.  Sort of a cursor event capture mode.
		void SetCapture(		// Returns nothing.
			short sActive);	// TRUE to activate, FALSE otherwise.

		// Processes a single event for all child hotboxes.
		// Non mouse events are ignored.
		// It is now up to the callback to check whether the input event
		// is used and decide whether to utilize the event.
		// Recurses on children.
		// This is called by the other Do().
		short Do(					// Returns priority of item that used event
										// or RHOT_NO_PRIORITY if none.
			RInputEvent*	pie);	// In:  Most recent user input event.
										// Out: Depends on callbacks.  Generally,
										// pie->sUsed = TRUE, if used.

		// Processes a single event for all child hotboxes.
		// Interface to above function that simply accepts Blue mouse event data.
		// NOTE:  If you use this Do() the RInputEvent passed to callbacks that
		// take such input, will have its lTime, sButtons, and sUsed set to 0.
		// If you do not use that type of callback, then there's no problem.
		// This is purely provided for backward compatibility.
		short Do(							// Returns priority of item that used event
												// or RHOT_NO_PRIORITY if none.
			short	sPosX,					// In:  X position for event.
			short	sPosY,					// In:  Y position for event.
			short sEvent)					// In:  A RSPiX Blue mouse button event
												// (see <Mac/Win/Etc>Blue.h).
			{
			// Create an event.
			RInputEvent	ie;
			ie.type		= RInputEvent::Mouse;
			ie.sPosX		= sPosX;
			ie.sPosY		= sPosY;
			ie.sButtons	= 0;
			ie.sEvent	= sEvent;
			ie.lTime		= 0;
			ie.sUsed		= FALSE;

			return Do(&ie);
			}

	public:	// Querries.

		short	IsActive(void)	// Returns TRUE if active, FALSE otherwise.
			{
			return m_sActive; 
			}

		short IsCapturing(void)	// Returns TRUE if capturing events, FALSE
										// otherwise.
			{ 
			return m_sCapture; 
			}

		// Get the child position equivalent coordinates.
		void GetChildPos(	// Returns nothing.
			short* psX,		// In:  Top-level position.
								// Out: Child position.
			short* psY);	// In:  Top-level position.
								// Out: Child position.    

		// Get the top position equivalent coordinates.
		void GetTopPos(	// Returns nothing.
			short* psX,		// In:  Child position.
								// Out: Top-level position.
			short* psY);	// In:  Child position.
								// Out: Top-level position.    

		RHot*	GetParent(void)	// Returns ptr to RHot that is parent to this Rhot.
			{
			return m_photParent;
			}

	public:	// Static functions.

	public:	// Internal public functions.

	protected:	// Internal functions.
		// Resets all members of this RHot.
		// Returns nothing.
		void Init(void);

		// Gets the list appropriate for this hotbox.
		ListHots* GetCaptureList(void);	// Returns Capture list appropriate for this hotbox.

	public:	// Static members.

	protected:	// Instantiable members.
		short		m_sActive;		// TRUE if active, FALSE otherwise.
		short		m_sCapture;		// TRUE, if capturing events.
		short		m_sPriority;	// Priority for hotbox.  Lower value     
										// equals closer to front (higher priority).
										// RHOT_NO_PRIORITY(default) indicates non-prioritized.
										// See CPP comment header in regards to specifics
										// of this value.
		RHot*		m_photParent;	// Pointer to parent RHot or NULL, if none.

	
	public:	// To be modified by the User.
		short		m_sX;				// The x-coordinate of this hotbox relative to
										// its parent.
		short		m_sY;				// The y-coordinate of this hotbox relative to
										// its parent.
		short		m_sW;				// The width of this hotbox.
		short		m_sH;				// The height of this hotbox.

		REventCall		m_ecUser;	// User callback on button events.
											// All callbacks can be used simultaneously.
		REventPosCall	m_epcUser;	// User callback on button events.  Includes
											// mouse position.  
											// All callbacks can be used simultaneously.
		InputEventCall	m_iecUser;	// User callback on input events.  Includes
											// a full RInputEvent.
											// All callbacks can be used simultaneously.
		ULONG		m_ulUser;			// User value passed to callbacks.

		SListHots	m_slistActiveChildren;	// List of active child RHots.
		ListHots		m_listChildren;			// List of all child RHots.
		// List of hots that always receive events.
		ListHots		m_listCapturing;			// List of all capturing child RHots
														// and self, if capturing.



	public:	// Static members.

	};

#endif	// HOT_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
