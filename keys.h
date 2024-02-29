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
// keys.h
// Project: Nostril (aka Postal)
//
// History:
//		03/31/97	JMI	Started.
//
//		07/06/97	JMI	Changed pu8ScanKey parm in KeyDescriptionToValue
//							call from a U8 to a short.
//							Also, changed g_apszButtonDescriptions to 
//							g_apszMouseButtonDescriptions.
//
//		10/10/97	JMI	Added g_apszJoyButtonDescriptions and
//							JoyButtonDescriptionToMask().
//
////////////////////////////////////////////////////////////////////////////////
//
//	Key stuff.  I'm not sure if this will ever amount to more than just the 
// descriptions.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef KEYS_H
#define KEYS_H

// Array of key descriptors.
extern char* g_apszKeyDescriptions[128];

// Array of button descriptors.
extern char* g_apszMouseButtonDescriptions[8];

// Array of joy button descriptors.
extern char* g_apszJoyButtonDescriptions[18];

// Given a string, returns the appropriate key code.
extern short KeyDescriptionToValue(	// Returns 0 on success.  Returns non-zero, if
												// key not found.
	char*		pszKeyDescriptor,			// In:  Description of key.
	U32*	psScanKey);					// Out: Key value.

// Given a string, returns the appropriate button mask.
extern short MouseButtonDescriptionToMask(	// Returns 0 on success.  Returns
															// non-zero, if description not 
															// found.
	char*		pszButtonDescriptor,					// In:  Description of button.
	U32*	psButtonMask);							// Out: Button mask.

extern short JoyButtonDescriptionToMask(	// Returns 0 on success.  Returns 
														// non-zero, if description not found.
	char*		pszButtonDescriptor,				// In:  Description of button.
	U32*	psButtonMask);						// Out: Button mask.

#endif	// KEYS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
