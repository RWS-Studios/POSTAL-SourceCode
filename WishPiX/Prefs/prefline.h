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
// prefline.h
//
//		12/11/96	JPW	RPrefsLine created to contain ini file lines, information
//							on the type of lines, and to help in processing of lines.
//		12/16/96	JPW	Fixed so it will work with the STL stuff that comes with 
//							MSVC 4.1 or newer.  Also fixed a few psz parameters that 
//							should have been const's.
//
//		03/28/97	JMI	Fixed so this'll work with MSVC 4.2.
//
//		05/17/97	JMI	Fixed so this'll work with MSVC 5.0 (removed rhetorical
//							RPrefsLine:: namespace name.
//
//		06/29/97 MJR	Replaced STL vector with an RSP list.  STL is an evil
//							entity that should be banished from the face of the earth.
//							Whoever suggested we use it should be shot.  (Good thing
//							I'm the president -- it's against the rules to shoot me.)
//
////////////////////////////////////////////////////////////////////////////////

#ifndef PREFLINE_H
#define PREFLINE_H

#include "Blue.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/CDT/flist.h"
#else
	#include "flist.h"
#endif

class RPrefsLine;

typedef RFList<RPrefsLine*> RPrefsLineList;


class RPrefsLine
	{
	public:
		typedef enum ePrefsLineType
			{
			Comment,
			Section,
			Variable
			};

	private:
		ePrefsLineType	m_Type;					// Type of line read from ini file
		char				*m_pszLine;				// Line read from ini file

	public:
		// Constructor.
		RPrefsLine (ePrefsLineType Type, const char *pszLine);

		// Destructor
		~RPrefsLine ();

		// Get a constant pointer to the Line of text.
		const char*	GetLine (void);

		// Get type of line.
		RPrefsLine::ePrefsLineType GetType();

		// Get the section name
		short GetSectionName(char *pszSection);
		
		// Get the variable name
		short GetVariableName(char *pszVariable);

		// Get the value of the variable
		short GetVariableValue(char *pszValue);

		// Set the value of the variable
		short SetVariableValue(const char *pszValue);
	};


#endif//PREFLINE_H