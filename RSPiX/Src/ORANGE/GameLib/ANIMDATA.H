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
//	ANIMDATA.H
//
// These are the animation data structs.
// They should probably be put in the PROJECT.H (or similar) file.
//
//*****************************************************************************

#ifndef _ANIMDATA_H
#define _ANIMDATA_H

#include "Blue.h"

#pragma warning (disable:4200) // disable warning about zero-length arrays

// A ZONE is basically just a rectangle.  The values are offsets from
// the hotspot to the edges of the rectangle.
typedef struct tag_ZONE
	{
	short sLeft;		// Offset from hotspot to left edge of zone
	short sTop;			// Offset from hotspot to top edge of zone
	short sRight;		// Offset from hotspot to right edge of zone
	short sBottom;		// Offset from hotspot to bottom edge of zone
	} ZONE;


// A ZONELIST is a list of ZONEs for a particular type.  In most cases
// a single ZONE is used, but complex shapes may require multiple ZONEs.
typedef struct tag_ZONELIST
	{
	short sNumZones;	// Number of zones in array
	ZONE aZones[];		// ZONE's (variable sized array!)
	} ZONELIST;


// A ZONESET is a list of zone types, with one entry per zone type used.
// The number of zones actually used is determined by the generated
// data and is defined in the associated .H file as "MAX_ZONETYPES".
// Actually, we may want this to be a runtime issue, so different
// games within a single product could have different max values.
// Note that the number of zonetypes determines the
// total size of this structure.
typedef struct tag_ZONESET
	{
	ZONELIST* apZoneLists[];	// ZONELIST's (variable sized array!)
	} ZONESET;


	
// Multiple frames will likely refer to the same image
typedef struct tag_IMAGE
	{
	short ucType;		// Image type
	short sWidth;		// Image width (bounding rectangle)
	short sHeight;		// Image height (bounding rectangle)
	// The next part is dependant on the image type.  Perhaps
	// we should use a UNION to define whatever parts of the
	// various image types we can.
	// For now, I just declare it as generic data bytes.
	UCHAR abData[];		// Image data (as per type)
	} IMAGE;


// A FRAME contains offsets that describe how the hotspot should be moved
// in order to get to the position at which the specified IMAGE should be
// displayed.  The "hold" value tells how many "ticks" this FRAME should
// be held for before moving on to the next frame.  The "repeat" value
// tells how often to repeat this frame (this is a built-in form of
// compression).  This repeat value is counted when determining the total
// number of frames in the animation.  The ZONESET describes the zones
// associated with this frame.
typedef struct tag_FRAME
	{
	short sOffsetX;
	short sOffsetY;
	short	sOffsetZ;
	short sHold;
	short sRepeat;			// Bit 15 could be used to mark key frames
								// If bit is set on frame 0, it would mean that
								// the animation uses inter-frame deltas.
	IMAGE* pImage;
	ZONESET* pZoneSet;
	} FRAME;




// Animation structure.  Note that the number of frames determines the
// total size of this structure.
typedef struct tag_ANIM
	{
	short sNumFrames;			// Number of frames
	FRAME aFrames[1];			// FRAME's (variable sized array!)
	} ANIM;


// Animation header structure.  Note that this structure is declaired 
// with an array of 1 aAnims, but it is really a variable sized array.
// Since we are allocating a block of memory for the while animation
// data file and just casting pointers to various parts of memory within
// that block, we don't need the size of the array.  The C compiler won't
//	let us declair variable sized arrays containing variable sized arrays, 
// so we can either do it by calling it size 1 and using is as a variable
// sized array, or by making the linker generate a constant for the total
// number of animations.  For now I've chosen to use the size 1 method.

typedef struct tag_ANIMSET
{
 	long lNumAnims;
	ANIM* apAnims[1];
} ANIMSET;


// Should we use the AIFF-like format where everything is in its
// own "chunk"?  The advantage I see is that we can then have a
// general-purpose file-viewing tool that wouldn't have to know
// specific file formats.  This might be useful for debugging.
// The disadvantage is that the chunk stuff takes up extra space
// (8 bytes per chunk) that could add up, especially on CD-i.
// A compromise may be to store the file that way but to strip
// out the chunk stuff as we load it into memory.  However,
// we would then have to fix the offsets to take that into account,
// which could be very difficult.  Storing the data with the
// offsets already set to ignore the chunk stuff would complete
// defeat the purpose of the chunks in the first place, since the
// file viewer wouldn't work anymore.
//
// Proposed four-character-codes are:
//		'alut' - animation lookup table
//		'anim' - animation data
//		'fram' - frame data
//		'zset' - zone set data
//		'zone' - zone data


// The file contains offsets which we want to convert into pointers.
// One way to do this is to write code that understands the file
// format and traverses all the data in it, adding the starting
// address to each of the offsets.  This has no memory/file overhead,
// but requires specific code for this purpose.  A general purpose
// alternative would be to have relocation data at the start of the
// file (I saw this on the Atari ST).  If all longs are long-aligned,
// then each bit could correspond to a long.  If the bit is 0 then
// we skip that long, if it is 1 we relocate it.  The size of the
// reloc data would be 1/32nd of the actual data.  A similar method
// could be used on a word-oriented bases, which would increase the
// overhead to 1/16th of the data.  Since the reloc data is temporary
// in nature, this shouldn't be a problem for CD-i.  This general-
// purpose approach has more overhead, but would work for all kinds
// of files without requiring separate code for each file type.

// If the same data is to be used for PC and Mac (we know CD-i gets
// its own) then we need to do byte-swapping on the data while we do
// the relocating.

// Because we are accessing these struct directly, we need to ensure that the
// alignment of these structs matches the way ALL of the compilers do it!!!!!!
// This brings up whether it might be better to have separate data files
// for PC and Mac.

// We decided to go for separate data animation data files for PC, Mac
// and, of course, CD-i.  This allows each one to have the appropriate
// alignment, padding, byte-order, etc.
//
// However, we probably can't afford the CD space to duplicate all
// of the background data, too.  Since backgrounds and sprite image data
// will be used by the same functions, the sprite image data must be
// stored the same as the bg data, which is to say in some standard
// format, probably PC-oriented.  This means that the image data must
// be in a separate file from the animation data.
// The offsets to images in the animation data will be offsets into
// the separate image file, which is easy since the linker generates
// both files.

// Animation lookup table.  Should we attempt to save space here?
// We could give the number corresponding to the first entry and
// the number of entries, which would allow us to leave off leading
// and trailing unused entries.  This is probably not usefull.


// Here's the data in the file:

/*

// This is animation lookup table
	dl	<number of entries in following table>
	dl	<offset to animation #0 or 0 if it doesn't exist in this file)
 	dl <offset to animation #1 or 0 if it doesn't exist in this file)
	(etc.)

// Data for an animation
	dw <number of frames>

// Data for frame 0 (only used once - loops go back to frame 1)
	dw	<start x>
	dw	<start y>
	dw	<start z>
	dw	<hold count>
	dw	<repeat count>
	dl	<offset to image>
	dl	<offset to zoneset, 0 if none>

// Data for frame 1 through n-1
	dw	<offset x>
	dw	<offset y>
	dw	<offset z>
	dw	<hold count>
	dw	<repeat count>
	dl	<offset to image>
	dl	<offset to zoneset, 0 if none>


// Sample access stuff.  We coerce generic pointers into pointers to specific
// structs.
	ANIM* pAnim = (long*)pLoadingAddress[1];
	short sNumFrames = pAnim->sNumFrames;
	FRAME* pFrame = pAnim->aFrames[3];

*/

#endif //ANIMDATA_H

//*****************************************************************************
// EOF
//*****************************************************************************
