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
//*****************************************************************************
//
// EVENT.CPP
//
// History:
//		02/09/95 BH		Started this module
//
//*****************************************************************************
//
//	FUNCTIONALITY
//
// The basic function of this module is to monitor the movement of the display
// area in order to determine whether any events associated with the 
// background map should be triggered.  If so, the routines which the
// aplication has associated with those events will be called.
//
// WHAT ARE EVENTS?
//
// Basically, events are attached to the background maps by the artists.  Each
// event consists of an x and y coordinate and a type, which is simply a
// number from 0 to 254 (255 is an end-of-list marker).  What each of
// those types "means" is up to the application.
//
// There are two basic types of events: those that are "triggered" when they
// get withing a certain distance of the display area, and those that are
// used as markers, which the application will explicitly look for at some
// point. 
//
// For example, one event may be used to indicate the player's starting
// position on the map.  Another event may be used to indicate each of the
// many places where a mutant penguin should appear.  Another may be used 
// to trigger a special feature, such as turning on color cycling.  Finally,
// an event can be used to simply mark a position, such as where sea-level is
// located.
//
//*****************************************************************************

#include <stdafx.h>
#include "System.h"
#include "event.h"
#include "univ.h"
		
extern CUniverse g_GameUniverse;

//*****************************************************************************
// Event Handlers (function pointers)
//*****************************************************************************

static void* event_handlers[] = 
{
	event_undefined,			// 0
	event_undefined,			// 1
	event_undefined,			// 2
	event_undefined,			// 3
	event_undefined,			// 4
	event_undefined,			// 5
	event_undefined,			// 6
	event_undefined,			// 7
	event_undefined,			// 8
	event_undefined,			// 9
	event_undefined,			// 10			
};

//*****************************************************************************
// Declair static members
//*****************************************************************************

EVENT* CEvent::m_pEventList;
short  CEvent::m_aIndexList[EVENT_MAX_COORD/EVENT_SPACING];
CList  CEvent::m_IgnoreList;

//*****************************************************************************
//
// Constructor
//
//*****************************************************************************

CEvent::CEvent()
{

}

//*****************************************************************************
//
// Destructor
//
//*****************************************************************************

CEvent::~CEvent()
{

}

//*****************************************************************************
//
// Reset
//
// Description:
//		Reset everything in this module.
//
//		The ignore list is cleared, and the default range for triggerable
//		event types is set.
//
// Input:
//		none
//
// Uses:
//		m_sEventTriggerMin/Max m_sEventConstMin/Max
//		m_IgnoreList
//
// Output:
//		none
//
// Return:
//		none
//
//*****************************************************************************

void CEvent::Reset()
{
	while (m_IgnoreList.Remove());			// Remove all items from the list

	m_sEventTriggerMin = 0;						// Set default trigger ranges
	m_sEventConstMin = 0;
	m_sEventTriggerMax = 255;
	m_sEventConstMax = 255;
	m_sOrientation = HORIZONTAL;				// Set default orientation
}

//*****************************************************************************
//
// SelectSet
//
// Description:
//		Selects a set of event data as the current set and builds an index
//		table which enables fast searching of events for the level.
//
// Input:
//
// Uses:
//
// Output:
//
// Return:
//
//*****************************************************************************

short CEvent::SelectSet(short sEventSet)
{
		// from the univ data - somewhere, get the event data
		// and create memory for the event data then point to it.
		// of if Univ creates the memory, just get the pointer.
		m_pEventData = g_GameUniverse.GetEvents();

		return SUCCESS;
}

//*****************************************************************************
//
// TriggerInit
//
// Description:
//		This routine triggers all events within the trigger area.  This area
//		is based around the display area.
//
// Input:
//		sDispX = x coordinate of display area's upper left corner
//		sDispY = y coordinate of display area's upper left corner
//
// Uses:
//
// Output:
//		none
//
// Return:
//		none
//
//*****************************************************************************

void CEvent::TriggerInit(short sDispX, short sDispY)
{
	// Save the display position
	m_sDispOldX = sDispX;
	m_sDispOldY = sDispY;

	// Determine boundries of trigger area.  Then scan the area and trigger
	// all events found within it.
	m_sScanYFrom = max(0, sDispY - EVENT_TRIGGER_TOP);
	m_sScanYTo = sDispY + GRIP_DISPLAY_H + EVENT_TRIGGER_BOTTOM;

	m_sScanXFrom = max(0, sDispX - EVENT_TRIGGER_LEFT);
	m_sScanXTo = sDispX + GRIP_DISPLAY_W + EVENT_TRIGGER_RIGHT;

	// Set to trigger from all directions
	m_sEventDir = EVENT_LEFT + EVENT_RIGHT + EVENT_UP + EVENT_DOWN;

	// Trigger all events in area
	Scan();
}

//*****************************************************************************
//
// Trigger
//
// Description:
//		Trigger any events which are "unconvered" by virtue of the display
//		area having moved.
//
//		First, we check the orientation and split the process into
//		left/right movement and up/down movement.  Next, we determine which
//		edge (left/right or top/bottom) is the leading edge.  Then we
//		calculate the coordinates of the tirgger area by adding onto that
//		edge's coordinates the appropriate trigger offset.  We do this for
//		both the previous and new position of the edge, and any event which
//		fall between those two values, inclusive, are triggered.
//
//		Note that in order to avoid multiple-triggers of the same event,
//		we have to make sure that the new trigger area does not overlap the
//		previous trigger area.  For example, when moving right, we don't
//		really want to go from the old right edge to the new right edge,
//		because if we do that same thing each time, then we'll be checking 
//		a particular coordinate again on the next move.  The solution for going
//		right is to go from the old right edge PLUS 1 to the new right edge.
//		This avoids the overlap.  A similar thing is done for each of the edges.
//
//		This routine also calls DecrementTimers to go through the ignore list
//
// Input:
//		sDispX = x coordinate of the display area's upper left corner
//		sDispY = y coordinate of the display area's upper left corner
//
// Uses:
//
// Output:
//		none
//
// Return:
//
//*****************************************************************************

short CEvent::Trigger(short sDispX, short sDispY)
{
 	// Decrement timers no matter what happens
	DecrementTimers();

	// Check for direction of movement
	enum Direction
	{
		UP, DOWN, LEFT, RIGHT, DOWN_RIGHT, DOWN_LEFT, UP_RIGHT, UP_LEFT, NONE
	};
		
	Direction eDir;

	if (sDispX != m_sDispOldX && sDispY != m_sDispOldY)
	{	//diagonal movement
		if (sDispY > m_sDispOldY)
		{	// down
			if (sDispX > m_sDispOldX)
				eDir = DOWN_RIGHT;
			else
				eDir = DOWN_LEFT;
		}
		else
		{	// up
			if (sDispX > m_sDispOldX)
				eDir = UP_RIGHT;
			else
				eDir = UP_LEFT;
		}
	}
	else
	{	// one direction only
		if (sDispY != m_sDispOldY)
		{	// up/down motion
			if (sDispY > m_sDispOldY)
				eDir = DOWN;
			else
				eDir = UP;
		}
		else
		{	// left/right motion
			if (sDispX > m_sDispOldX)
				eDir = RIGHT;
			else
				eDir = LEFT;
		}
	}

	switch (eDir)
	{
	 	case UP:
			m_sEventDir = EVENT_UP;
			From_Top(sDispY);
			To_Top(m_sDispOldY);
			From_Left(m_sDispOldX);
			To_Right(m_sDispOldX);
			Scan();			
			break;

		case DOWN:
			m_sEventDir = EVENT_DOWN;
			From_Bottom(m_sDispOldY);
			To_Bottom(sDispY);
			From_Left(m_sDispOldX);
			To_Right(m_sDispOldX);
			Scan();
			break;

		case RIGHT:
			m_sEventDir = EVENT_RIGHT;
			From_Right(m_sDispOldX);
			To_Right(sDispX);
			From_Top(m_sDispOldY);
			To_Bottom(m_sDispOldY);
			Scan();
			break;

		case LEFT:
			m_sEventDir = EVENT_LEFT;
			From_Left(sDispX);
			To_Left(m_sDispOldX);
			From_Top(m_sDispOldY);
			To_Bottom(m_sDispOldY);
			Scan();
			break;

		case DOWN_RIGHT:
			m_sEventDir = EVENT_DOWN + EVENT_RIGHT;
			// Area 1
			From_Bottom(m_sDispOldY);
			To_Bottom(m_sDispOldY);
			From_Left(sDispX);
			To_Right(sDispX);
			Scan();	
			// Area 2
			From_Right(m_sDispOldX);
			To_Right(m_sDispOldX);
			From_Top(sDispY);
			To_Bottom(m_sDispOldY);
			Scan();		
			break;

		case DOWN_LEFT:
			m_sEventDir = EVENT_DOWN + EVENT_LEFT;
			// Area 1
			From_Bottom(m_sDispOldY);
			To_Bottom(sDispY);
			From_Left(sDispX);
			To_Right(sDispX);
			Scan();	
			// Area 2
			From_Left(sDispX);
			To_Left(m_sDispOldX);
			From_Top(sDispY);
			To_Bottom(m_sDispOldY);
			Scan();		
			break;

		case UP_RIGHT:
			m_sEventDir = EVENT_UP + EVENT_RIGHT;
			// Area 1
			From_Top(sDispY);
			To_Top(m_sDispOldY);
			From_Left(sDispX);
			To_Right(sDispX);
			Scan();
			// Area 2
			From_Right(m_sDispOldX);
			To_Right(sDispX);
			From_Top(m_sDispOldY);
			To_Bottom(sDispY);
			Scan();
			break;

		case UP_LEFT:
			m_sEventDir = EVENT_UP + EVENT_LEFT;
			// Area 1
			From_Top(sDispY);
			To_Top(m_sDispOldY);
			From_Left(sDispX);
			To_Right(sDispX);
			Scan();
			// Area 2
			From_Left(sDispX);
			To_Left(m_sDispOldX);
			From_Top(m_sDispOldY);
			To_Bottom(sDispY);
			Scan();
			break;

		case NONE:

			break;
	}

	// Save display position for next time
	m_sDispOldX = sDispX;
	m_sDispOldY = sDispY;


	return SUCCESS;					//or perhaps some other value
}

//*****************************************************************************
//
// Ignore
//
// Description:
//		Add the specified id into the "ignore" list.  As long as the id
//		is left in the ignore list, the event associated with it will not
//		be triggered.
//
//		A "handle" is returned by this routine, and the application must use
//		this "handle" when calling Unignore().
//
//		As a special feature, an event id of $ffff will be ignored (meaning
//		it won't be added to the list).  This may be helpful in those
//		situations where the application is already calling this routine but
//		sometimes doesn't ahve a valid event id to send to it.  It can
//		basically create a "dummy" event id by using $ffff . 
//
//		NOTE:  This routine does not check to see if the id being added to
//		this list is already on the list.  Therefore, the application can 
//		add an id more than once if it so desires.  Of course, each of those
//		id's would need to be removed from the list in order for the event
//		to be triggered again.
//
// Input:
//		sEventID = unique id (as passed to event handler by this module, or
//					  $ffff if inserting a "dummy" value)
//		sEventTimer = MUST BE $FFFF OR $FFFE (EVENT_PERMANENT OR EVENT_TEMPORARY)
//
// Uses:
//
// Output:
//
// Return:
//		CEvent* pointer to the event - "handle"
//
//*****************************************************************************

void CEvent::Ignore(EVENT* pEvent)
{
	m_IgnoreList.Add((LISTNODE) pEvent);
}

//*****************************************************************************
//
// Unignore
//
// Description:
//
// Input:
//
// Uses:
//
// Output:
//
// Return:
//		none
//
//*****************************************************************************

void CEvent::Unignore(EVENT* pEvent)
{
	m_IgnoreList.Remove((LISTNODE) pEvent);
}


//*****************************************************************************
//
// Search
//
// Description:
//		Search for a particular event type in the current set's event list.
//		If found, all of the relevant data about that event is returned
//
// Input:
//		sEventType = type of event to search for.
//
// Uses:
//		m_pEventData array
//
// Output:
//		none
//
// Return:
//		EVENT* pEvent which serves as its ID.  In addition the structure
//		contains x, y, type, status, and timer.
//
//*****************************************************************************
 
EVENT* CEvent::Search(short sEventNum)
{
	short s = 0;

	while (m_pEventData[s].sType != sEventNum && m_pEventData[s].sType >= 0)
		s++;
	if (m_pEventData[s].sType < 0)
		return NULL;
	else
		return &(m_pEventData[s]);		 	
}

//*****************************************************************************
//
// SearchNext
//
// Description:
//		Similar to Search, but finds the next event of the same type given
//		a pointer to the event.  For example, if you wanted to find all of
//		the events of type $02, you would call Search(2) to get a pointer to
//		the first event of that type in the level.  If you wanted to find the
//		next occurance of event type $02, you would call SearchNext(pEvent)
//		where pEvent is the pointer to the event type $02 that was returned from
//		the Search() function.
//
// Input:
//		pEvent = pointer to 
//
// Uses:
//		m_pEventData array
//
// Output:
//		none
//
// Return:
//		EVENT* pEvent
//
//*****************************************************************************

EVENT* CEvent::SearchNext(EVENT* pEvent)
{
	short s = 0;
	
	while (m_pEventData[s].sType != pEvent->sType && 
	       m_pEventData[s].sType >= 0             &&
			 &(m_pEventData[s]) == pEvent
	      )
		s++;

	if (m_pEventData[s].sType < 0)
		return NULL;
	else
		return &(m_pEventData[s]);
}

//*****************************************************************************
//
// CreateIndexes
//
// Description:
//		Create table of index values for current set of event data.
//
//		The index table contains indexes into the event data.  It's purpose
//		is to speed up searches through the event data by giving us a
//		starting position within the event data which is near (or at) the
//		event data we are looking for.
//
// 	The tables are indexed using the coordinate divided by 16 and return
//		the index of the first event whose coordinate is equal to or greater
//		than the coordinate AND'ed with $fff0.
//
// 	Algorithm
//
//		coord = 0;
//		event = 0;
//		index = 0;
//
//		do
//		{
//			while (array[event] < coord)
//				event = event + 1;
//			event_index[index] = event;
//			index = index + 1;
//			
//			coord = coord + EVENT_SPACING
//		}
//		while (coord < EVENT_INDEX_MAX);
//
// Input:
//		sOrientation = orientation of section, vertical or horizontal
//	
// Uses:
//		pEventData
//
// Output:
//		m_pIndexList;
//
// Return:
//		none
//
//*****************************************************************************

void CEvent::CreateIndexes(short sOrientation)
{
	short sCoord = 0;
	short sEvent = 0;
	short sIndex = 0;

	if (sOrientation == HORIZONTAL)
	{
		do
		{
			while (m_pEventData[sEvent].sX < sCoord)
				sEvent++;
			m_aIndexList[sIndex++] = sEvent;
			sCoord += EVENT_SPACING;

		} while (sCoord < EVENT_INDEX_MAX);		
	}
	else // VERTICAL
	{
		do
		{
			while (m_pEventData[sEvent].sY < sCoord)
				sEvent++;
			m_aIndexList[sIndex++] = sEvent;
			sCoord += EVENT_SPACING;

		} while (sCoord < EVENT_INDEX_MAX);
	} 	
}


//*****************************************************************************
//
// Scan
//
// Description:
//		Scan for events in the specified range.
//
//		This routine doesn't know whether it is searching for x or y
//		coordinates.  The m_sEventScanFrom and m_sEventScanTo variables are
//		simply set to the start and end of the range to scan through.
//
//		We need to find all events which are defined by the 4 corners, 
//		m_sScanXFrom, m_sScanXTo, m_sScanYFrom, m_sScanYTo inclusive.
//
//		To find out where in the list to start, we look into a table of indexes
//		which has been conveniently constructed ahead of time.  We index this
//		table with m_sScanFrom divided by 16, and the result is the index to the
//		first event whose coordinate is equal to or greater than m_sEventScanFrom
//		AND'ed with $fff0.
//
//		Starting with that index, we go through the events looking for coordinates
//		that are between m_sScanFrom and m_sScanTo, inclusive.  Note that
//		we don't have to do a separate check for the end of the event array because
//		the end is marked by an $ffff, which will always be greater than the range
//		we're looking for, and will thus end the scan.
//
//		For every event in the range, we call its event handler (see CallHandler
//		for more information.)
//
// Input:
//		sOrientation = HORIZONTAL or VERTICAL
//
// Uses:
//		m_sScanXFrom
//		m_sScanXTo
//		m_sScanYFrom
//		m_sScanYTo
//		m_pEventData
//		
// Output:
//
// Return:
//
//*****************************************************************************

short CEvent::Scan()
{
	short s;
	short sEventIndex;

	if (m_sOrientation == VERTICAL)
	{	// Vertical - aIndexList is sorted on Y
		s = m_sScanYFrom / EVENT_SPACING;
		sEventIndex = m_aIndexList[s];
		while (m_pEventData[sEventIndex].sY < m_sScanYTo)
		{
			if (m_pEventData[sEventIndex].sY > m_sScanYFrom)
			{
				if (m_pEventData[sEventIndex].sX >  m_sScanXFrom &&
				    m_pEventData[sEventIndex].sX <= m_sScanXTo)
					CallHandler(sEventIndex);	// Trigger event
			}
			sEventIndex++;
		}
	}
	else
	{	// Horizontal - aIndexList is sorted on X
		s = m_sScanXFrom / EVENT_SPACING;
		sEventIndex = m_aIndexList[s];
		while (m_pEventData[sEventIndex].sX < m_sScanXTo)
		{
		 	if (m_pEventData[sEventIndex].sX > m_sScanXFrom)
			{
			 	if (m_pEventData[sEventIndex].sY >  m_sScanYFrom &&
				    m_pEventData[sEventIndex].sY <=	m_sScanYTo)
					CallHandler(sEventIndex);	// Trigger event
			}
			sEventIndex++;
		}
	}
	return SUCCESS;				//or perhaps some other value
}

//*****************************************************************************
//
// CallHandler
//
// Description:
//		Call an event's handler, but only if:
//		1.	The event type is within the application-definable
//			"triggerable range"
//		2.	The event's unique id is not on the "ignore" list
//		3.	The display has moved in a direction which agrees with that
//			defined by the event status.
//
//		We are passed only the index into the event data.  We must first
//		get all of the events data and put it into the member variables
//		which are then used by the event handler.  Along the way, we
//		generate the event's unique id.
//
// 	We check the unique id against the list of id's to be ignored, and
//		if we find the id, then we ignore it.
//
//		If we don't find the id, then we call the event's handler by getting
//		its address from the list in which the application has inserted all
//		of the event handler addresses.  We do a simulated indirect call to
//		the routine, and it returns to us here when it's done.
//
// Input:
//		sEventIndex = index into event array
//
// Uses:
//
// Output:
//
// Return:
//
//*****************************************************************************

short CEvent::CallHandler(short sEventIndex)
{
//-----------------------------------------------------------------------------
// Get event's data
//
// Check if event's type is within "triggerable" range.  The minimum is
//	the lowset triggerable event type and the maximum is the highest event
// type PLUS 1!
//-----------------------------------------------------------------------------

	if ((m_pEventData[sEventIndex].sType < m_sEventConstMin || 
	     m_pEventData[sEventIndex].sType > m_sEventConstMax) &&
		 (m_pEventData[sEventIndex].sType < m_sEventTriggerMin ||
		  m_pEventData[sEventIndex].sType > m_sEventTriggerMax))
		return EVENT_NOT_IN_RANGE;

//-----------------------------------------------------------------------------
// Check if event's id appears in ignore list.  If it does, check the timer.
//	If the timer is negative, the event is still active, so ignore it.  
// If the timer is zero, unignore the event and retrigger it.  
// If the timer is positive, ignore the event.  (Event_Trigger does the 
// decrementing).
//-----------------------------------------------------------------------------

	EVENT* pEvent = (EVENT*) m_IgnoreList.GetHead();
	BOOL 	 bFound = FALSE;

	while (pEvent != NULL && !bFound)
	{
		if (pEvent == &(m_pEventData[sEventIndex]))
			bFound = TRUE;
		else
			pEvent = (EVENT*) m_IgnoreList.GetNext();
	}

	if (bFound && pEvent->sIgnoreTimer != 0)
		return EVENT_ON_IGNORE_LIST;

//-----------------------------------------------------------------------------
// Check whether the display has moved in a direction which agrees with that
// defined by the event status.
//
// NOTE: As long as the direction of scrolling overlaps with the direction
// defined by m_sEventDir, the event should be triggered.  For example, if
// m_sEventDir has EVENT_RIGHT bit set and the display has moved diagonally
// right and up, the event should be triggered.
//
//	If there are no directional flags set for this event then assume it can 
// be triggered by any direction.  If directional flags are set, then see if
// any of those directions match the scroll direction.  If either of these
// is true, then trigger this event.
//
// The event handler function pointers are stored in the table event_handler
// at the top of this file.  
//------------------------------------------------------------------------------


//	if ((pEvent->sFlags & m_sEventDir) || !(pEvent->sFlags & EVENT_DIR_MASK))
//		(event_handlers[pEvent->sType])();

	return SUCCESS;
}

//*****************************************************************************
//
// DecrementTimers
//
// Description:
//		Decrement all timers in the Ignore list.  The "real" timers are those
//		with positive values.  -1 (EVENT_PERMANENT) or -2 (EVENT_TEMPORARY)
//		are flags only.
//
// Input:
//		none
//
//	Uses:
//		m_IgnoreList and the list of EVENTS that it contains as nodes
//		Decrements m_IgnoreList->pEvent->sTimer
//
// Output:
//		none
//
// Return:
//		none
//
//*****************************************************************************

void CEvent::DecrementTimers()
{
 	EVENT* pEvent = (EVENT*) m_IgnoreList.GetHead();

	while (pEvent)
	{
	 	if (pEvent->sTimer > 0)
			pEvent->sTimer++;
		pEvent = (EVENT*) m_IgnoreList.GetNext();
	}
}

//*****************************************************************************
//
// event_undefined
//
// Description:
//		Place holder function that is placed in the event handler table for
//		event numbers which have no assigned function.
//
//*****************************************************************************

void event_undefined()
{

}


//*****************************************************************************
// EOF
//*****************************************************************************

