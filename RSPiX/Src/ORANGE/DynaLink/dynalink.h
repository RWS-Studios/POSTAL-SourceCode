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
// dynalink.h
// 
// History:
//		01/10/96	JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// This object implements linking at run time.  We do this by keeping an array
// of function pointers in a class whose constructor fills an array element
// with a function pointer.  We are guaranteed to be able to access the
// function safely (i.e., we know it's already allocated) b/c it is a static
// member of the constructing class.
//
// Use the macros below to instantiate your class and to provide easy user
// implementation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef DYNALINK_H
#define DYNALINK_H

//////////////////////////////////////////////////////////////////////////////
// Includes.
//////////////////////////////////////////////////////////////////////////////
#include "common/bdebug.h"
	
//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
// Use this macro to instantiate your class and its array of function ptrs.
// FunctionTypedef is a typedef of a pointer to your function type.
// FriendClass is a class to befriend this one (allowing it, and only it,
//		to access the function array.
// MaxElements is the maximum number of functions in your array list.  It must
//		be 1 or greater.
#define LINKINSTANTIATE(FunctionTypedef, FriendClass, MaxElements)	\
	FunctionTypedef																\
		CDynaLink<FunctionTypedef, FriendClass, MaxElements>::ms_afp[MaxElements]	\
			= { NULL, }

// Use this macro to create a macro for your users to use to link.
// You can use this function so you don't have to remember any template 
// declaration crap.  Create a macro in your header for your users that calls
// LINKLATE similar to the following:
// #define LINKME(pUserFunc, lUserFuncIndex)	\
//		LINKLATE([Your Function Typedef], [Your Friend Class], [Max functions], \
//					pUserFunc, lUserFuncIndex)
// FunctionTypedef is a typedef of a pointer to your function type.
// FriendClass is a class to befriend this one (allowing it, and only it,
//		to access the function array.
// MaxElements is the maximum number of functions in your array list.  It must
//		be 1 or greater.
// pUserFunc is a pointer to the user's function.
// lUserFuncIndex is the index in the function array to be filled with 
//		pUserFunc.
// Note that dl##FunctionTypedef##lUserFuncIndex creates a name for the 
//	CDynaLink that is specific for each element of the array which means
// one module can link more than one function and allows easy identification
// of the CDynaLink during compile, link, and debug steps.  This means,
// however, you cannot put anything for lUserFuncIndex that would make an
// invalid variable name (i.e., a + 1, MACRO(ARG), etc. are sux).
#define LINKLATE(FunctionTypedef, FriendClass, MaxElements, pUserFunc, lUserFuncIndex)	\
	static CDynaLink<FunctionTypedef, FriendClass, MaxElements>	\
		dl##FunctionTypedef##lUserFuncIndex(pUserFunc, lUserFuncIndex)

// To access a function from your array.
// FunctionTypedef is a typedef of a pointer to your function type.
// FriendClass is a class to befriend this one (allowing it, and only it,
//		to access the function array.
// MaxElements is the maximum number of functions in your array list.  It must
//		be 1 or greater.
// lUserFuncIndex is the index in the function array to be filled with 
//		pUserFunc.
// Returns a pointer to the user function.
#define GETLINKFUNC(FunctionTypedef, FriendClass, MaxElements, lUserFuncIndex)	\
	(CDynaLink<FunctionTypedef, FriendClass, MaxElements>::ms_afp[lUserFuncIndex])	

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
template <class FUNCTYPEPTR, class FRIEND, long lMax> class CDynaLink
	{
	public:
		// Constructor Especial.
		CDynaLink(FUNCTYPEPTR pfn, long lIndex)
			{
			ASSERT(lIndex < lMax);
			ASSERT(ms_afp[lIndex] == NULL);
			ms_afp[lIndex]	= pfn;
			}

		friend FRIEND;

	protected:		
		static FUNCTYPEPTR			ms_afp[lMax];
	};

#endif // DYNALINK_H
