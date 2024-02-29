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
// ATTRIBUTE.H
//
// Created on 10/03/96 JMI
// Implemented	03/14/97 JRD
//
// 03/18/97 JRD	Started this file to remove the Postal specific 
//						interpretation of RAttribute out of the orange
//						layer to allow a second attribute grid with different
//						interpretations.
//
//////////////////////////////////////////////////////////////////////
//  After Postal, this class will be renamed to something more general
//  such as "RMultiGrid", since attribute data can be stored in many ways

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "BLUE/Blue.h"
	#include "ORANGE/File/file.h"
#else
	#include "Blue.h"
	#include "file.h"
#endif // PATHS_IN_INCLUDES

 #define ATTRIBUTE_MAP_COOKIE 0x4d525441 //Looks like "ATRM" in the file
 #define ATTRIBUTE_CURRENT_VERSION 6

//////////////////////////////////////////////////////////////////////
//  Here are the Postal attribute masks, grouped by word.  
//////////////////////////////////////////////////////////////////////
//
// NOTE:  FOR SPEED REASONS, do NOT SHIFT values unless ABSOLUTELY
// necessary.  This is not needed for flags, or numeric fields when
// only relative values are important.  If a field TRULY needs to
// have absolute values, it should be stored in the correct place in
// the attribute mask.
//
// ALSO:  AVOID using fields which need both OFFSETS and SCALING!
//			(To create a signed field, just subtract half)
//
// NOTE:  MACROS for VALUES are for comparison ONLY - they cannot be 
//			used for ANDing and ORing
//
//////////////////////////////////////////////////////////////////////
//	 Postal Attribute Map ONE:
//////////////////////////////////////////////////////////////////////

// BITS	0-7, signed numeric field
#define ATTRIBUTE_HEIGHT_MASK  0x00FF	// Mask of height bits.

// BITS 8-11, unsigned numeric field
#define ATTRIBUTE_LAYER_MASK 0x0F00		// Mask of layer cue

// BIT 12, flag
#define ATTRIBUTE_CLIFF	0x1000			// cliff attribute

// BIT 13, flag
#define ATTRIBUTE_NOT_WALKABLE 0x2000	// Flag for no-walk areas.

// BIT 14, flag
#define ATTRIBUTE_LIGHT_EFFECT 0x4000	// Flag for light

//////////////////////////////////////////////////////////////////////
//	 Postal Attribute Map TWO:
//////////////////////////////////////////////////////////////////////

// BITS 0-10 OPENED FOR ALL POSTAL DUDES!
// That's 2048 possibilities opened up for the postal crew!

// BITS 11-12, unsigned numeric field
#define	ATTRIBUTE_RESERVED1_MASK	0x1800	// I reserve two for later

// BITS 13-14, unsigned numberic field
#define	ATTRIBUTE_FLUID_MASK		0x6000	// Mask of fluids
#define	ATTRIBUTE_BLOOD_VALUE	0x2000	// Value of BLOOD
#define	ATTRIBUTE_OIL_VALUE		0x4000	// Value of GASOLINE
#define	ATTRIBUTE_WATER_VALUE	0x6000	// Value of WATER

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//  The goal was to make getting the field of an attribute have ZERO
//	 cost to the user.  One change is that since BIT15 is removed
//  from the attribute on decompression, all values deal with 
//  SIGNED shorts.  The meaning is uneffected, but it provides greater
//  compatibility and options with comparing external values.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//  Here are Postal specific interpretations of fields:
//  Add as many as you like, but consolidate them here.
//  All will be considered DYNAMIC -> changeable like the wind!
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//  Somewhat general macros
//////////////////////////////////////////////////////////////////////
// interpret a flag as 1 or zero
inline	short	GetFlag(short sMaskedValue)
	{
	// faster than : ? assembly.
	if (sMaskedValue) return 1;
	return 0;
	}

// interpret a field as a signed range:
inline	short	GetField(short sMaskedValue,short sLowValue)
	{
	return sMaskedValue + sLowValue;
	}

// interpret a field as a signed, scaled range:
// FIRST scale it, THEN offset it!
// DO NOT shift left more than 2 bits!
//
inline	short	GetFieldMul(short sMaskedValue,short sLowValue,short sMulBits)
	{
	return (sMaskedValue<<sMulBits) + sLowValue;
	}

// interpret a field as a signed, scaled range:
// FIRST scale it, THEN offset it!
// DO NOT shift right more than 8 bits!
//
inline	short	GetFieldDiv(short sMaskedValue,short sLowValue,short sDivBits)
	{
	return (sMaskedValue>>sDivBits) + sLowValue;
	}

//////////////////////////////////////////////////////////////////////
//  very specific macros (try to design attribute position to optimize!)
//////////////////////////////////////////////////////////////////////
//  As always, these are PRE-MASKED fields!

// range is -512 to +511 from 8 bits
//
inline	short	GetHeight(short sMaskedValue)
	{
	return	GetFieldMul(sMaskedValue,-512,2);
	}

// range is 0 to 15
//
inline	short	GetLayer(short sMaskedValue))
	{
	return (sMaskedValue >> 8);
	}

//-------//-----//
#endif	// EOF //
//-------//-----//
