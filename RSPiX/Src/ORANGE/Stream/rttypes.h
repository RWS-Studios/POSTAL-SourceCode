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
#ifndef RTTYPES_H
#define RTTYPES_H
//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
// This is the number of types supported (it can be up to 65536).
// Keep in mind that for every type each class stores 12 bytes.  That's
// 4K for 256 types.
#define NUM_TYPES	 256

// These are the types reserved for each stream type.
// Currently only the first 256 [0..255] are utilized by dispatch.  But up to
// 64K can be used.  It should be a USHORT defined as 0x????.
// Add your type here.  Do not use one someone else did.  Update this file
// immediately after you add yours.  Don't be a bunghole. :)

#define RT_TYPE_RTFINFO			0x0000	// RTF Info.
#define RT_TYPE_FILEIMAGE		0x0001	// File image.
#define RT_TYPE_FLIC				0x0002	// Real Time FLX (FLI, FLC, FLU).
#define RT_TYPE_SND				0x0003	// Real Time SND (PCM).
#define RT_TYPE_VIDC				0x0004	// Real Time Windows Video CODEC.


// Flags for usFlags field of chunk header.
// Use them to determine your chunk types.
#define RT_FLAG_INIT			0x01	// Typically the first or header chunk.
#define RT_FLAG_LAST			0x02	// Typically the last chunk.
#define RT_FLAG_TAG			0x04	// Typically an alert/callback motivation.
#define RT_FLAG_RESERVED1	0x08	// Reserved.  DO NOT USE!
#define RT_FLAG_RESERVED2	0x10	// Reserved.  DO NOT USE!
#define RT_FLAG_RESERVED3	0x20	// Reserved.  DO NOT USE!
#define RT_FLAG_USER2		0x40	// User preference.
#define RT_FLAG_USER1		0x80	// User preference.


#endif // RTTYPES_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
