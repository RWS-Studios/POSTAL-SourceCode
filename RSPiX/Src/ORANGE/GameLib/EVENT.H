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
// EVENT.H
//
//*****************************************************************************

#ifndef event_h
#define event_h

#include "grip.h"
#include "list.h"

#define	HORIZONTAL			1
#define  VERTICAL				0

#define	EVENT_IGNORES		50		//This determines how many events 
											//will fit on the ignore list.  The
											//larger this value, the slower the
											//searches through the list will be.
											//Can be set to any value keeping
											//in mind memory and speed limitations

#define	EVENT_SPACING		16		//This value is used when generating
											//indexes into the event data.  It
											//determines the spacing between
											//the coordinate associated with one
											//index and the coordinae associated
											//with the next index.  By setting this
											//to a smaller value, you can cut down
											//on the number of compares required
											//to find events within a given
											//coordinate range, but at the same
											//time, you increase the size of the
											//index table.  The size of the table
											//is given by EVENT_MAX_COORD divided
											//by this value.

#define	EVENT_MAX_COORD	4096	//This is the maximum value that
											//this module will encounter in
											//the event data.  It can handle
											//higher values, but the search 
											//times for any values above
											//this one will not have the benefit
											//of the index table, and hence
											//will be very slow
											//This value cannot exceed 32767

#define	EVENT_TRIGGER_TOP		32	//Top of trigger region
#define	EVENT_TRIGGER_BOTTOM	32	//Bottom of trigger region
#define 	EVENT_TRIGGER_LEFT	32	//Left side of trigger region
#define	EVENT_TRIGGER_RIGHT	32	//Right side of trigger region	

#define	EVENT_LEFT				(short) 0x8000 //%1000000000000000
#define	EVENT_RIGHT				(short) 0x4000 //%0100000000000000
#define	EVENT_UP					0x2000 //%0010000000000000
#define	EVENT_DOWN				0x1000 //%0001000000000000
#define	EVENT_DIR_MASK			0xf000 //%1111000000000000

#define 	EVENT_INDEX_MAX		255

#define	EVENT_PERMANENT		-1
#define 	EVENT_TEMPORARY		-2

#define	EVENT_NOT_IN_RANGE	10
#define	EVENT_ON_IGNORE_LIST	11



void event_undefined();				//null event handler function

typedef struct tag_EVENT
{
 	short sX;					// From data file	= x position
	short sY;					// From data file = y position
	short sType;				// From data file = application assigned id number
	short sFlags;				// From data file = directional or optional trigger
	short	sTimer;				// Used when the event is on the ignore list
	short sIgnoreTimer;		// Created & used in CEvent
} EVENT;

class CEvent
{
protected:
	short	m_sScanYFrom;
	short m_sScanYTo;
	short	m_sScanXFrom;
	short	m_sScanXTo;
	short	m_sEventTriggerMin;
	short m_sEventTriggerMax;
	short	m_sEventConstMin;
	short m_sEventConstMax;
	short m_sDispOldX;
	short m_sDispOldY;
	short m_sOrientation;
	short	m_sEventDir;
	EVENT* m_pEventData;


	static EVENT*	m_pEventList;		// Currently selected event set for this level
	static short 	m_aIndexList[EVENT_MAX_COORD/EVENT_SPACING];
	static CList	m_IgnoreList;	

	short SelectSet(short sEventSet);
	void  TriggerInit(short sDispX, short sDispY);
	short Trigger(short sDispX, short sDispY);
	void  CreateIndexes(short sOrientation);
	short Scan();
	short CallHandler(short sIndex);
	void  DecrementTimers();

	
	inline void From_Bottom(short s)
	{
		m_sScanYFrom = s + GRIP_DISPLAY_H + EVENT_TRIGGER_BOTTOM + 1;
	}

	inline void To_Bottom(short s)
	{
		m_sScanYTo = s + GRIP_DISPLAY_H + EVENT_TRIGGER_BOTTOM;
	}

	inline void From_Top(short s)
	{
		m_sScanYFrom = max(0, s - EVENT_TRIGGER_TOP);
	}

	inline void To_Top(short s)
	{
		m_sScanYTo = max(0, s - (EVENT_TRIGGER_TOP + 1));
	}

	inline void From_Left(short s)
	{
		m_sScanXFrom = max(0, s - EVENT_TRIGGER_LEFT);
	}

	inline void To_Left(short s)
	{
		m_sScanXTo = max(0, s - (EVENT_TRIGGER_LEFT + 1));
	}

	inline void From_Right(short s)
	{
		m_sScanXFrom = s + GRIP_DISPLAY_W + EVENT_TRIGGER_RIGHT + 1;
	}

	inline void To_Right(short s)
	{
		m_sScanXTo = s + GRIP_DISPLAY_W + EVENT_TRIGGER_RIGHT;
	}


public:
	// Default Constructor
	CEvent();

	// Destructor
	~CEvent();

	// Reset - warm start the module
	void Reset();

	// Add Set
	short AddSet(short sSetNum);

	// Select Set 
	short SelectSet();

	// TriggerInit - Call at start of level to trigger on-screen events
	void TriggerInit();

	// Trigger - Called every time a new event is "uncovered"
	void Trigger();

	// Ignore - Put event on the ignore list
	void Ignore(EVENT* pEvent);

	// Unignore - remove event from the ignore list
	void Unignore(EVENT* pEvent);

	// Search - search for event in selected event list
	EVENT* Search(short sEventNum);

	// SearchNext - search for next event of same type in the list
	EVENT* SearchNext(EVENT* pEvent);

	// SearchAll - search for event in all event lists
	short SearchAll(short sEventNum);
};
	


#endif //event_h
