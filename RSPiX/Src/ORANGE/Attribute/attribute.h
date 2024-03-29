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
//////////////////////////////////////////////////////////////////////
//
// ATTRIBUTE.H
//
// Created on	10/03/96 BRH
// Implemented	10/03/96 BRH
//
//	10/03/96 BRH	Started this class for use in the Postal
//						demo to provide a way to load the attribute
//						map and to access the data.
//
// 10/31/96 BRH	Changed CAttributeMap to RAttributeMap for the 
//						new naming convention.
//
// 11/20/96 BRH	Changed to new file version now that there are
//						detail maps in addition to the block map.
//
// 01/23/97 BRH	Changed walkable attribute bit to allow bit
//						space for another alpha layer.  Changed the
//						version number due to this change.
//
//	02/03/97 BRH	Added Load(RFile*) in addition to the Load(char*)
//
//	03/13/97	JMI	Added ATTRIBUTE_HEIGHT_MASK macro.
//
//////////////////////////////////////////////////////////////////////

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#ifdef PATHS_IN_INCLUDES
	#include "BLUE/Blue.h"
	#include "ORANGE/File/file.h"
#else
	#include "Blue.h"
	#include "file.h"
#endif // PATHS_IN_INCLUDES

 #define ATTRIBUTE_MAP_COOKIE 0x4d525441 //Looks like "ATRM" in the file
 #define ATTRIBUTE_CURRENT_VERSION 5

 #define ATTRIBUTE_NOT_WALKABLE 0x2000 // was 0x0800
 #define ATTRIBUTE_LIGHT_EFFECT 0x4000

 #define ATTRIBUTE_HEIGHT_MASK  0x00FF	// Mask of height bits.

//////////////////////////////////////////////////////////////////////
//
// RAttributeMap class
//
// This class makes it easier to load and access attribute maps for
//	games.  There is a separate utility that converts Photoshop files
// (.PSD) to attribute maps.  It looks for a layer called
// "attributes" and reads the R, G and B channels to get the 
// attribute data.  The attribute bits are set in that utility.  
// This class simply returns the values stored in the attribute map
// at different locations.  It has functions for returning attributes
// for a single point or an OR mask of all points within a rectangle.
//
//////////////////////////////////////////////////////////////////////

class RAttributeMap
{
	public:
		int32_t m_lWidth;
		int32_t m_lHeight;
		int32_t m_lWorldWidth;
		int32_t m_lWorldHeight;
		int16_t m_sScaleX;
		int16_t m_sScaleY;
		int16_t m_sNumDetailMaps;
		int16_t m_sBlockDataSize;

		// General Constructor
		RAttributeMap();

		// Contstuctor that opens the given map file
		RAttributeMap(char* pszFilename);

		// General Destructor
		~RAttributeMap();

		// Load function - to load a map
		int16_t Load(char* pszFilename);

		// Load function that takes an open RFile pointer
		int16_t Load(RFile* prf);

		// Single point attribute
		uint16_t GetAttribute(int32_t lX, int32_t lY);

		// Rectangle attribute
		uint16_t GetAttribute(int32_t lTop, int32_t lBottom, int32_t lLeft, int32_t lRight);

		// Get just the low 8 bits (flags) from the last GetAttribute Call
		uint8_t GetFlags()
			{return m_ucFlags;};
		
		// Get the maximum height from the last GetAttribute Call
		uint8_t GetMaxHeight()
			{return m_ucMaxHeight;};

		// Get the minimum height from the last GetAttribute Call
		uint8_t GetMinHeight()
			{return m_ucMinHeight;};

	private:
		// pointer to map buffer
		uint16_t* m_pusMap;

		// pointer to detail map buffer
		uint16_t* m_pusDetailMap;

		// The attribute stored since the last GetAttribute Call
		uint16_t m_usLastAttribute;

		// The Max height stored since the last GetAttribute Call
		uint8_t m_ucMaxHeight;

		// The Min height stored since the last GetAttribute Call
		uint8_t m_ucMinHeight;

		// The flags stored since the last GetAttribute Call
		uint8_t m_ucFlags;

		// Allocate buffer for map
		int16_t AllocateMap(uint32_t ulSize, uint32_t ulDetailMapSize);
			
	public:
		// Deallocate buffer for map
		void FreeMap();

};


#endif //ATTRIBUTE_H

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
