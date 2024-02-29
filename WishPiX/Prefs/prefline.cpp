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
// prefline.cpp
// Project: Soon-to-be-RSPiX
//
// History:
//		12/11/96	JPW	RPrefsLine created to contain ini file lines, information
//							on the type of lines, and to help in processing of lines.
//		12/16/96	JPW	Fixed so it will work with the STL stuff that comes with 
//							MSVC 4.1 or newer.  Also fixed a few psz parameters that 
//							should have been const's.
//
//		05/08/97	JMI	Added conditions for compiler versions' STL
//							differences (namely "list" vs. "list.h").
//							Also, changed #include <rspix.h> to #include "RSPiX.h".
//
//		06/29/97 MJR	Replaced STL vector with an RSP list.  STL is an evil
//							entity that should be banished from the face of the earth.
//							Whoever suggested we use it should be shot.  (Good thing
//							I'm the president -- it's against the rules to shoot me.)
//
//		07/10/97 MJR	Removed TRACE() that occured if there was nothing following
//							an entry's '='.
//
//		08/27/97	JMI	Now Var names and Section names can contain spaces (note,
//							though, that leading and trailing spaces are ignored for
//							user's convenience).
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "Blue.h"
#include "prefline.h"

////////////////////////////////////////////////////////////////////////////////
// Handy inline to parse before and after an equals into two strings.
// The whitespace directly preceding and postceding.
////////////////////////////////////////////////////////////////////////////////
inline short GetVar(	// Returns 0 on success.
	char*	pszLine,		// In:  Line to parse.
	char*	pszVar,		// Out: Var name, if not NULL.
	char* pszVal)		// Out: Val, if not NULL.
	{
	short	sRes	= 0;	// Assume success.

	short	j, i, k;
	// Copy variable name to out string
	for (j = 0, i = 0; pszLine[i] != '\0' && pszLine[i] != '='; i++, j++)
		{
		if (pszVar)
			{
			pszVar[j] = pszLine[i];
			}
		}

	if (pszVar)
		{
		// Remove trailing whitespace.
		for (k = j - 1; isspace(pszVar[k]) && k > 0; k--) ;

		pszVar[k + 1] = '\0';
		}

	if (pszLine[i] == '\0')
		{
		TRACE("GetVar(): Missing '=' in line: '%s'\n", pszLine);
		sRes = 2;
		}
	else
		{
		if (pszVal)
			{
			// Find first non-space char after '='
			for (i++; pszLine[i] != '\0' && isspace(pszLine[i]); i++)
				;
			// Did we find find anything after '='
			if (pszLine[i] == '\0')
				{
// 7/10/97 MJR - Removed this TRACE() because we often use entries with nothing after the '='
//				TRACE("GetVar(): Badly formed variable syntax.\n");
				sRes = 3;
				}
			else
				// Copy variable value to out string
				strcpy(pszVal, &pszLine[i]);
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////

RPrefsLine::RPrefsLine (RPrefsLine::ePrefsLineType Type, const char *pszLine)
	{
	m_Type = Type;
	m_pszLine = new char[strlen (pszLine) + 1];
	assert (m_pszLine);
	strcpy (m_pszLine, pszLine);

	return;
	}

////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
RPrefsLine::~RPrefsLine ()
	{
	delete [] m_pszLine;
	m_pszLine = NULL;
	return;
	}

////////////////////////////////////////////////////////////////////////////////
// Method to get a constant pointer to the Line of text.
////////////////////////////////////////////////////////////////////////////////
const char*	RPrefsLine::GetLine (void)
	{
	return (m_pszLine);
	}

////////////////////////////////////////////////////////////////////////////////
// Method to get type of line.
////////////////////////////////////////////////////////////////////////////////
RPrefsLine::ePrefsLineType RPrefsLine::GetType()
	{
	return (m_Type);
	}

////////////////////////////////////////////////////////////////////////////////
// Get the section name. returns 0 on success
////////////////////////////////////////////////////////////////////////////////
short RPrefsLine::GetSectionName(char *pszSection)
	{
	short sRes = 0;
	if (m_Type != Section)
		{
		TRACE("RPrefsLine::GetSectionName(): Not a section line.\n");
		sRes = 1;
		}
	else
		{
		short i, j;
		// Find index of first char is section name
		for (i = 0; (m_pszLine[i] != '\0') &&
				(isspace(m_pszLine[i]) || (m_pszLine[i] == '[')); i++)
			;
		// Make sure there even is somthing after the '[' beside space
		if (m_pszLine[i] == '\0')
			sRes = 2;
		else
			{
			// Copy section name to out string
			for (j = 0; (m_pszLine[i] != '\0') && (m_pszLine[i] != ']'); i++, j++)
				pszSection[j] = m_pszLine[i];

			j--;

			// Remove trailing spaces.
			while (isspace(pszSection[j]) )
				j--;

			pszSection[j + 1] = '\0';
			}
		}
	if (sRes != 0)
		strcpy(pszSection, "");
	return (sRes);
	}
		
////////////////////////////////////////////////////////////////////////////////
// Get the variable name. returns 0 on success
////////////////////////////////////////////////////////////////////////////////
short RPrefsLine::GetVariableName(char *pszVariable)
	{
	short sRes = 0;
	if (m_Type != Variable)
		{
		TRACE("RPrefsLine::GetVariableName(): Not a variable line.\n");
		sRes = 1;
		}
	else
#if 0
		{
		short i, j;
		// Find index of first non-space char
		for (i = 0; m_pszLine[i] != '\0' && isspace(m_pszLine[i]); i++)
			;
		if (m_pszLine[i] == '\0')
			{
			TRACE("RPrefsLine::GetVariableName(): Badly formed variable name.\n");
			sRes = 2;
			}
		else
			{
			// Copy variable name to out string
			for (j = 0; m_pszLine[i] != '\0' && !isspace(m_pszLine[i]) &&
					m_pszLine[i] != '='; i++, j++)
				pszVariable[j] = m_pszLine[i];
			pszVariable[j] = '\0';
			}
		}
	if (sRes != 0)
		strcpy(pszVariable, "");
#else
		sRes	= GetVar(m_pszLine, pszVariable, NULL);
#endif
	return (sRes);
	}

////////////////////////////////////////////////////////////////////////////////
// Get the value of the variable. returns 0 on success
////////////////////////////////////////////////////////////////////////////////
short RPrefsLine::GetVariableValue(char *pszValue)
	{
	short sRes = 0;

	// Make sure the prefs line is a variable
	if (m_Type != Variable)
		{
		TRACE("RPrefsLine::GetVariableValue(): Not a variable line.\n");
		sRes = 1;
		}
	else
#if 0
		{
		short i;
		// Find index of '=' char
		for (i = 0; m_pszLine[i] != '\0' && m_pszLine[i] != '='; i++)
			;
		if (m_pszLine[i] == '\0')
			{
			TRACE("RPrefsLine::GetVariableName(): Missing '=' in line: '%s'\n", m_pszLine);
			sRes = 2;
			}
		else
			{
			// Find first non-space char after '='
			for (i++; m_pszLine[i] != '\0' && isspace(m_pszLine[i]); i++)
				;
			// Did we find find anything after '='
			if (m_pszLine[i] == '\0')
				{
// 7/10/97 MJR - Removed this TRACE() because we often use entries with nothing after the '='
//				TRACE("RPrefsLine::GetVariableName(): Badly formed variable syntax.\n");
				sRes = 3;
				}
			else
				// Copy variable value to out string
				strcpy(pszValue, &m_pszLine[i]);
			}
		}
	if (sRes != 0)
		strcpy(pszValue, "");
#else
		sRes	= GetVar(m_pszLine, NULL, pszValue);
#endif
	return (sRes);
	}

////////////////////////////////////////////////////////////////////////////////
// Set the value of the variable
////////////////////////////////////////////////////////////////////////////////
short RPrefsLine::SetVariableValue(const char *pszValue)
	{
	short	sRes = 0;
	char	pszLine[128], pszVariable[64];

	ASSERT(pszValue);
	// Make sure the prefs line is a variable
	if (m_Type != Variable)
		{
		TRACE("RPrefsLine::SetVariableValue(): Not a variable line.\n");
		sRes = 1;
		}
	else
		{
#if 0
		short i;
		for(i = 0; (m_pszLine[i] != '\0') && !isspace(m_pszLine[i]) &&
				m_pszLine[i] != '='; i++)
			pszVariable[i] = m_pszLine[i];
		pszVariable[i] = '\0';
#else
		sRes	= GetVar(m_pszLine, pszVariable, NULL);
		if (sRes == 0)
#endif
			{
			ASSERT(m_pszLine);
			delete [] m_pszLine;
			m_pszLine = NULL;
			sprintf(pszLine, "%s = %s", pszVariable, pszValue);
			m_pszLine = new char[strlen(pszLine) + 1];
			strcpy(m_pszLine, pszLine);
			}
		}
	return (sRes);
	}
