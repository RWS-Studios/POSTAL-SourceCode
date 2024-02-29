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
// logtab.h
// Project: Nostril (aka Postal)
//
// History:
//		05/17/97 MJR	Started.
//
//		05/19/97	JMI	Added forward declaration of CLogTab.  Note that normally 
//							you don't need to do this but, in VC 5.0, the friend 
//							directive seems to need to know whether or not the class           
//							being befriended is templated or not so that it'll parse it
//							correctly.  I'm not sure if this is a parser bug or new          
//							stricter ANSI requirements.  /shrug.                                                  
//
//		05/27/97 MJR	Added logging feature.
//
//		05/27/97 MJR	Added comment token.
//							Added operators (=, <, >, !, *).
//
//		07/21/97 BRH/MJR Fixed table matching bug on last column in logic table.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef LOGTAB_H
#define LOGTAB_H

#include <stdio.h>
#include <ctype.h>
#include "RSPiX.h"

class RFile;

// Name of log file (if there is one)
#define LOGTAB_LOGFILE							"logtablog.log"

// Error codes
#define LOGTAB_ERR_NO_SUCH_VAR				-10
#define LOGTAB_ERR_DUP_VAR						-11
#define LOGTAB_ERR_INVALID_VAR_TEXT			-12
#define LOGTAB_ERR_BAD_RANGE_VAR_TEXT		-13
#define LOGTAB_ERR_NOT_OUTPUT_VAR			-14
#define LOGTAB_ERR_VAR_NOT_FOUND				-15
#define LOGTAB_ERR_TOO_MANY_VARS				-16
#define LOGTAB_ERR_NO_ROWS_AFTER_HEADINGS	-17
#define LOGTAB_ERR_NO_INPUTS					-18
#define LOGTAB_ERR_NO_OUTPUT_SEPARATOR		-19
#define LOGTAB_ERR_NO_OUTPUTS					-20
#define LOGTAB_ERR_EXTRA_COLUMNS				-21
#define LOGTAB_ERR_TOO_FEW_COLUMNS			-22
#define LOGTAB_ERR_TOO_MANY_ROWS				-23
#define LOGTAB_ERR_UNEXPECTED_EOF			-24
#define LOGTAB_ERR_READ_ERROR					-25
#define LOGTAB_ERR_TEXT_TOO_LONG				-26

// Token for "comment" (everything after it is ignored up to the end of the row)
#define LOGTAB_COMMENT_TOKEN		"//"

// Symbol in logic table files used to separate input columns from output columns.
#define LOGTAB_OUTPUT_SEPARATOR	"->"

// Symbol in logic table files for "don't care"
#define LOGTAB_DONT_CARE			"*"

// The default name given to all vars, just in case someone forgets to assign
// a real name in their derived var class.
#define LOGTABVAR_NONAME	"<NO NAME!>"

// For "internal" use only when calling rspMsgBox().  This whole approach will
// change once I switch over to an error callback.
#define LOGTAB_MSG								RSP_MB_BUT_OK | RSP_MB_ICN_STOP, "Logic Table Loader" 

// Forward declaration of CLogTab.  Note that normally you don't need to do this
// but, in VC 5.0, the friend directive seems to need to know whether or not
// the class being befriended is templated or not so that it'll parse
// it correctly.  I'm not sure if this is a parser bug or new stricter
// ANSI requirements.  /shrug.
template <class usertype>
class CLogTab;

////////////////////////////////////////////////////////////////////////////////
// This is the base-class implimentation for Logic Table Variables.
////////////////////////////////////////////////////////////////////////////////
template <class usertype>
class CLogTabVar
	{
	// Make logic table a friend
	friend class CLogTab<usertype>;

	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		static CLogTabVar* ms_pHead;							// Head of linked list of variables

		char* m_pszName;											// Variable name

		short m_sMaxVal;											// Maximum value

		bool m_bSettable;											// Whether val is settable (true) or not (false)

		short m_sNumStrings;										// Number of strings (0 if number based)
		char** m_papszStrings;									// Pointer to array of pointers to strings

		short m_sOutputWidth;									// Width of output in characters

		short m_sRefCount;										// Reference count (how many logic tables)

		CLogTabVar* m_pNext;										// Pointer to next variable
		CLogTabVar* m_pPrev;										// Pointer to prev variable

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CLogTabVar(void)
			{
			// Point name to default value so we can detect errors and so we can use
			// this pointer without worrying about whether it's valid.
			m_pszName = LOGTABVAR_NONAME;

			// Set maximum value to default of 0
			m_sMaxVal = 0;

			// Default to value not being settable
			m_bSettable = false;

			// Init string stuff
			m_sNumStrings = 0;
			m_papszStrings = 0;

			// Set default output width
			m_sOutputWidth = strlen(LOGTABVAR_NONAME);

			// Clear reference count
			m_sRefCount = 0;

			// Insert at front of linked list
			m_pNext = ms_pHead;
			m_pPrev = 0;
			if (ms_pHead)
				ms_pHead->m_pPrev = this;
			ms_pHead = this;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CLogTabVar()
			{
			// Check reference count and issue warning if necessary
			if (m_sRefCount > 0)
				{
				TRACE("CLogTabVar::~CLogTabVar(): Destroying '%s', which is still being used by CLogTab(s) (refcount = %hd)!\n", m_pszName, (short)m_sRefCount);
				ASSERT("Destroying CLogTabVar that's being used by one or more CLogTab's (see trace for details)" == 0);
				}

			// Remove from linked list
			if (m_pNext)
				m_pNext->m_pPrev = m_pPrev;
			if (m_pPrev)
				m_pPrev->m_pNext = m_pNext;
			else
				ms_pHead = m_pNext;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Find the variable with the specified name
		////////////////////////////////////////////////////////////////////////////////
		static
		short FindVar(												// Returns 0 if successfull, non-zero otherwise
			char* pszName,											// In:  Variable name to find
			CLogTabVar** ppVar);									//	Out: Pointer to variable (if found)

		////////////////////////////////////////////////////////////////////////////////
		// Increment reference count
		////////////////////////////////////////////////////////////////////////////////
		void IncRefCount(void)
			{
			m_sRefCount++;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Decrement reference count
		////////////////////////////////////////////////////////////////////////////////
		void DecRefCount(void)
			{
			m_sRefCount--;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Convert text into value
		////////////////////////////////////////////////////////////////////////////////
		short TextToVal(											// Returns 0 if successfull, non-zero otherwise
			char* pszText,											// In:  Text to convert
			short* psVal);											// Out: Value (only if successfull)

		////////////////////////////////////////////////////////////////////////////////
		// Convert value into text.  Resulting text will be truncated if necessary to
		// fit specified maximum text length.  If the text is truncated, the final
		// character will be a '#' to indicate the truncation.
		//
		// NOTE: Specified text length MUST BE AT LEAST 10!  This helps avoid all sorts
		// of niggling little problems.
		////////////////////////////////////////////////////////////////////////////////
		short ValToText(											// Returns 0 if successfull, non-zero otherwise
			short sVal,												// In:  Value to convert
			char* pszText,											// Out: Text (only if successfull)
			short sMaxText);										// In:  Maximum text length (must be >= 10)

		////////////////////////////////////////////////////////////////////////////////
		// Get maximum value (range is 0 to N, where this returns N)
		////////////////////////////////////////////////////////////////////////////////
		short GetMax(void)										// Returns maximum value
			{
			return m_sMaxVal;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get name
		////////////////////////////////////////////////////////////////////////////////
		char* GetName(void)										// Returns name
			{
			return m_pszName;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get current value (range is 0 to N, where N is returned by GetMax())
		////////////////////////////////////////////////////////////////////////////////
		virtual short GetVal(									// Returns current value
			usertype user)											// In:  User type passed here
			{
			TRACE("CLogTabVar::GetVal(): '%s' is likely missing a derived-class GetVal()!\n", m_pszName);
			return 0;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Set current value (range is 0 to N, where N is returned by GetMax())
		////////////////////////////////////////////////////////////////////////////////
		virtual void SetVal(
			usertype user,											// In:  User type passed here
			short sVal)												// In:  Value to set
			{
			if (m_bSettable)
				TRACE("CLogTabVar::SetVal(): '%s' is likely missing a derived-class SetVal()!\n", m_pszName);
			else
				TRACE("CLogTabVar::SetVal(): '%s' is not settable!\n", m_pszName);
			}
	};


////////////////////////////////////////////////////////////////////////////////
// Static's for CLogTabVar
////////////////////////////////////////////////////////////////////////////////
template <class usertype>
CLogTabVar<usertype>* CLogTabVar<usertype>::ms_pHead = 0;


////////////////////////////////////////////////////////////////////////////////
// Find the variable with the specified name
////////////////////////////////////////////////////////////////////////////////
//static
template <class usertype>
short CLogTabVar<usertype>::FindVar(				// Returns 0 if successfull, non-zero otherwise
	char* pszName,											// In:  Variable name to find
	CLogTabVar** ppVar)									//	Out: Pointer to variable (if found)
	{
	short sResult = 0;

	// Scan through linked list looking for specified name
	short sFound = 0;
	CLogTabVar* p = ms_pHead;
	while (p)
		{
		if (rspStricmp(pszName, p->m_pszName) == 0)
			{
			*ppVar = p;
			sFound++;

			// In debug mode we don't want to stop (we're looking for duplicates)
			#ifndef _DEBUG
				break;	// Only stop in release mode
			#endif
			}
		p = p->m_pNext;
		}

	// If not found, it's an error
	if (sFound == 0)
		{
		sResult = LOGTAB_ERR_NO_SUCH_VAR;
		TRACE("CLogTabVar::FindName(): Cannot find variable named '%s'\n", pszName);
		}

	// In debug mode, we check for duplicate names
	#ifdef _DEBUG
		if (sFound > 1)
			{
			sResult = LOGTAB_ERR_DUP_VAR;
			TRACE("CLogTabVar::FindName(): Found %hd variables named '%s'!\n", (short)sFound, pszName);
			}
	#endif

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Convert text into value
////////////////////////////////////////////////////////////////////////////////
template <class usertype>
short CLogTabVar<usertype>::TextToVal(				// Returns 0 if successfull, non-zero otherwise
	char* pszText,											// In:  Text to convert
	short* psVal)											// Out: Value (only if successfull)
	{
	short sResult = 0;

	// Check if this var is string-based or number-based
	if (m_sNumStrings > 0)
		{
		// Scan through array of strings looking for a match
		short s;
		for (s = 0; s < m_sNumStrings; s++)
			{
			if (rspStricmp(pszText, m_papszStrings[s]) == 0)
				{
				*psVal = s;
				break;
				}
			}
		// If no matches were found, it's an error
		if (s == m_sNumStrings)
			{
			sResult = LOGTAB_ERR_INVALID_VAR_TEXT;
			TRACE("CLogTabVar::TextToVal(): '%s' is not a valid setting for '%s'!\n", pszText, m_pszName);
			}
		}
	else
		{
		// Make sure text consists only of digits
		short sLen = strlen(pszText);
		for (short s = 0; s < sLen; s++)
			{
			if (!isdigit(pszText[s]))
				{
				sResult = LOGTAB_ERR_INVALID_VAR_TEXT;
				TRACE("CLogTabVar::TextToVal(): '%s' is not a valid setting for '%s'!\n", pszText, m_pszName);
				break;
				}
			}
		if (sResult == 0)
			{
			// Convert to value, then validate range
			short sTmp = (short)atoi(pszText);
			if ((sTmp >= 0) && (sTmp <= m_sMaxVal))
				*psVal = sTmp;
			else
				{
				sResult = LOGTAB_ERR_BAD_RANGE_VAR_TEXT;
				TRACE("CLogTabVar::TextToVal(): '%hd' is out of range for '%s'!\n", (short)sTmp, m_pszName);
				}
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Convert value into text.  Resulting text will be truncated if necessary to
// fit specified maximum text length.  If the text is truncated, the final
// character will be a '#' to indicate the truncation.
//
// NOTE: Specified text length MUST BE AT LEAST 10!  This helps avoid all sorts
// of niggling little problems.
////////////////////////////////////////////////////////////////////////////////
template <class usertype>
short CLogTabVar<usertype>::ValToText(				// Returns 0 if successfull, non-zero otherwise
	short sVal,												// In:  Value to convert
	char* pszText,											// Out: Text (only if successfull)
	short sMaxText)										// In:  Maximum text length (must be >= 10)
	{
	ASSERT(sMaxText >= 10);

	short sResult = 0;

	// Check if this var is string-based or number-based
	if (m_sNumStrings > 0)
		{
		// If value is in range, lookup its string equivalent
		if ((sVal >= 0) && (sVal < m_sNumStrings))
			{
			if (strlen(m_papszStrings[sVal]) < sMaxText)
				strcpy(pszText, m_papszStrings[sVal]);
			else
				{
				strncpy(pszText, m_papszStrings[sVal], sMaxText - 2);
				pszText[sMaxText - 2] = '#';
				pszText[sMaxText - 1] = 0;
				}
			}
		else
			{
			sResult = LOGTAB_ERR_BAD_RANGE_VAR_TEXT;
			TRACE("CLogTabVar::ValToText(): Value (%hd) is out of range for '%s'!\n", (short)sVal, m_pszName);
			}
		}
	else
		{
		// Convert value to text equivalent (we know that a short can't take up
		// any more than 6 characters, and that's with a negative sign in front).
		sprintf(pszText, "%hd", (short)sVal);
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Logic Table Class
////////////////////////////////////////////////////////////////////////////////
template <class usertype>
class CLogTab
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:
		typedef enum
			{
			MaxVars = 32,			// Limited only by memory considerations
			MaxRows = 100,			// Limited only by memory considerations
			MaxTextLen = 32		// A reasonable value
			};

		typedef enum
			{
			Equal,
			Not,
			Less,
			Greater,
			DontCare
			} Operand;

		typedef struct
			{
			Operand operand;
			short sEntry;
			} Cell;

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		short m_sRows;												// Number of rows in table
		short m_sInVars;											// Number of input vars
		short m_sOutVars;											// Number of output vars
		short m_sTotalVars;										// Number of vars

		CLogTabVar<usertype>* m_apVars[MaxVars];			// Array of pointers to vars

		Cell m_cellTable[MaxRows][MaxVars];					// Table of input and output cells

		bool m_bLogFileError;
		FILE* m_fpLog;
		short m_sLogLastRow;

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CLogTab()
			{
			m_sRows = 0;
			m_sInVars = 0;
			m_sOutVars = 0;
			m_sTotalVars = 0;

			m_bLogFileError = false;
			m_fpLog = NULL;
			m_sLogLastRow = -1;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CLogTab()
			{
			// Close log file
			if (m_fpLog != NULL)
				fclose(m_fpLog);

			FreeVars();
			}

		////////////////////////////////////////////////////////////////////////////////
		// Evaluate table
		////////////////////////////////////////////////////////////////////////////////
		short Evaluate(											// Returns 0 if successfull, non-zero otherwise
			usertype user,											// In:  User type passed here
			bool bLog);												// In:  Whether to log (true) or not (false)

		////////////////////////////////////////////////////////////////////////////////
		// Dump contents of logic table to file.
		////////////////////////////////////////////////////////////////////////////////
		void DumpToLog(void);

		////////////////////////////////////////////////////////////////////////////////
		// Load from current position of already-open file
		////////////////////////////////////////////////////////////////////////////////
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile);											// In:  RFile to load from

		////////////////////////////////////////////////////////////////////////////////
		// Save to current position of already-open file
		////////////////////////////////////////////////////////////////////////////////
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile);											// In:  RFile to save to

	private:
		////////////////////////////////////////////////////////////////////////////////
		// Free vars associated with this table
		////////////////////////////////////////////////////////////////////////////////
		void FreeVars(void)
			{
			// Decrement vars' reference counts since we're no longer using them
			for (short s = 0; s < m_sTotalVars; s++)
				m_apVars[s]->DecRefCount();
			}

		////////////////////////////////////////////////////////////////////////////////
		// (See .cpp for details)
		////////////////////////////////////////////////////////////////////////////////
		short ReadEntry(
			RFile* pFile,											// In:  RFile to read from
			char* pszEntry,										// Out: Entry is returned in here
			short sMaxEntrySize,									// In:  Maximum size of entry
			bool* pbEndOfTable,									// Out: true if this is last entry in table
			bool* pbEndOfRow,										// Out: true if this is last entry in row
			char* pcSave);											// I/O: Save char from last call (or 0 if none)
	};


////////////////////////////////////////////////////////////////////////////////
// Evaluate table
////////////////////////////////////////////////////////////////////////////////
template <class usertype>
short CLogTab<usertype>::Evaluate(
	usertype user,											// In:  User type passed here
	bool bLog)												// In:  Whether to log (true) or not (false)
	{
	short sResult = 0;

	// If we're asked to log but a log error occurred previously, then don't log
	if (bLog && m_bLogFileError)
		bLog = false;

	// If we're logging but we haven't started the log file yet, then do it now.
	if (bLog && (m_fpLog == NULL))
		{
		m_fpLog = fopen(LOGTAB_LOGFILE, "wt");
		if (m_fpLog != NULL)
			DumpToLog();
		else
			m_bLogFileError = true;
		}

	// Clear array of "cached" vals.  By caching these values, we avoid having
	// to re-evaluate the vars each time we need them, thereby saving lots of time.
	short sVals[MaxVars];
	for (short s = 0; s < m_sInVars; s++)
		sVals[s] = -1;

	// Scan table looking for a row that matches all the way across
	char acBuf[256];
	bool bRowMatched = false;
	for (short sRow = 0; sRow < m_sRows; sRow++)
		{
		// Optimistically default to matching cell (it's faster)
		bool bCellMatched = true;
		short sVar;
		for (sVar = 0; bCellMatched && (sVar < m_sInVars); sVar++)
			{
			// Get the entry from the table
			short sEntry = m_cellTable[sRow][sVar].sEntry;

			// Evaluate entry as per operand type
			switch(m_cellTable[sRow][sVar].operand)
				{
				case Equal:
					// If value isn't cached, then ask var for it now
					if (sVals[sVar] == -1)
						sVals[sVar] = m_apVars[sVar]->GetVal(user);

					// If value doesn't match entry, it fails
					if (sVals[sVar] != sEntry)
						bCellMatched = false;
					break;

				case Not:
					// If value isn't cached, then ask var for it now
					if (sVals[sVar] == -1)
						sVals[sVar] = m_apVars[sVar]->GetVal(user);

					// If value doesn't match entry, it fails
					if (sVals[sVar] == sEntry)
						bCellMatched = false;
					break;

				case Less:
					// If value isn't cached, then ask var for it now
					if (sVals[sVar] == -1)
						sVals[sVar] = m_apVars[sVar]->GetVal(user);

					// If value is not less than entry, it fails
					if (sVals[sVar] >= sEntry)
						bCellMatched = false;
					break;

				case Greater:
					// If value isn't cached, then ask var for it now
					if (sVals[sVar] == -1)
						sVals[sVar] = m_apVars[sVar]->GetVal(user);

					// If value is not greater than entry, it fails
					if (sVals[sVar] <= sEntry)
						bCellMatched = false;
					break;

				case DontCare:
					// Don't need to do anything
					break;

				default:
					TRACE("CLogTab::Evaluate(): Unknown operand!\n");
					break;
				}
			}

		// If we reached the last input var, then this is row is a complete match!
		if (sVar == m_sInVars && bCellMatched)
			{
			// Set flag
			bRowMatched = true;

			// Set output vars as indicated by table entries
			for (short sVar = 0; sVar < m_sOutVars; sVar++)
				{
				// If operand is NOT don't care, then set var to specified entry
				if (m_cellTable[sRow][m_sInVars + sVar].operand != DontCare)
					m_apVars[m_sInVars + sVar]->SetVal(user, m_cellTable[sRow][m_sInVars + sVar].sEntry);
				}

			// If we're logging and this isn't the same row as we logged last time...
			if (bLog && (sRow != m_sLogLastRow))
				{
				// Save new row for next time
				m_sLogLastRow = sRow;

				// Write out the values of all vars
				fprintf(m_fpLog, "Matched row #%hd: ", (short)sRow);
				for (short sVar = 0; sVar < m_sTotalVars; sVar++)
					{
					fprintf(m_fpLog, "%s=", m_apVars[sVar]->GetName());
					if (sVar < m_sInVars)
						{
						short sEntry = sVals[sVar];
						if (sEntry != -1)
							{
							m_apVars[sVar]->ValToText(sEntry, acBuf, sizeof(acBuf));
							fprintf(m_fpLog, "%s,  ", acBuf);
							}
						else
							fprintf(m_fpLog, "<ignored>,  ");
						}
					else
						{
						if (m_cellTable[sRow][sVar].operand != DontCare)
							{
							m_apVars[sVar]->ValToText(m_cellTable[sRow][sVar].sEntry, acBuf, sizeof(acBuf));
							fprintf(m_fpLog, "%s", acBuf);
							}
						else
							fprintf(m_fpLog, "<dontcare>");
						m_apVars[sVar]->ValToText(m_apVars[sVar]->GetVal(user), acBuf, sizeof(acBuf));
						fprintf(m_fpLog, "(%s),  ", acBuf);
						}
					// Put a separator between the input and output vars
					if ((sVar + 1) == m_sInVars)
						fprintf(m_fpLog, "  =>  ");
					}
				// Write out the row number we matched
				fprintf(m_fpLog, "\n");
				}

			// Stop row loop
			break;
			}

		}

	// If we're logging and no rows matched...
	if (bLog && !bRowMatched)
		{
		// Reset last row so next matching row will get logged
		m_sLogLastRow = -1;

		// Show input vars
		fprintf(m_fpLog, "No rows matched!  ");
		for (short sVar = 0; sVar < m_sInVars; sVar++)
			{
			fprintf(m_fpLog, "%s=", m_apVars[sVar]->GetName());
			short sEntry = sVals[sVar];
			if (sEntry != -1)
				{
				m_apVars[sVar]->ValToText(sEntry, acBuf, sizeof(acBuf));
				fprintf(m_fpLog, "%s,  ", acBuf);
				}
			else
				fprintf(m_fpLog, "<ignored>,  ");
			}
		fprintf(m_fpLog, "\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Dump contents of logic table to file.
////////////////////////////////////////////////////////////////////////////////
template <class usertype>
void CLogTab<usertype>::DumpToLog(void)
	{
	fprintf(m_fpLog, "This is a LogTab logging file.\n\n");
	
	fprintf(m_fpLog, "Contents of the logic table:\n");

	// Write out first row (names of vars)
	for (short sVar = 0; sVar < m_sTotalVars; sVar++)
		{
		fprintf(m_fpLog, "%s   ", m_apVars[sVar]->GetName());

		// Put a separator between the input and output vars
		if ((sVar + 1) == m_sInVars)
			fprintf(m_fpLog, "  =>  ");
		}
	fprintf(m_fpLog, "\n");

	// Write out rest of table
	for (short sRow = 0; sRow < m_sRows; sRow++)
		{
		fprintf(m_fpLog, "Row #%hd: ", (short)sRow);
		for (short sVar = 0; sVar < m_sTotalVars; sVar++)
			{
			// Get the entry from the table
			Operand op = m_cellTable[sRow][sVar].operand;
			switch(op)
				{
				case Equal:
					fprintf(m_fpLog, "=");
					break;

				case Not:
					fprintf(m_fpLog, "!");
					break;

				case Less:
					fprintf(m_fpLog, "<");
					break;

				case Greater:
					fprintf(m_fpLog, ">");
					break;

				case DontCare:
					fprintf(m_fpLog, "*              ");
					break;

				default:
					TRACE("CLogTab::Evaluate(): Unknown operand!\n");
					break;
				}
			if (op != DontCare)
				{
				char acBuf[256];
				m_apVars[sVar]->ValToText(m_cellTable[sRow][sVar].sEntry, acBuf, sizeof(acBuf));
				fprintf(m_fpLog, "%s   ", acBuf);
				}
			}
		fprintf(m_fpLog, "\n");
		}

	fprintf(m_fpLog, "\n");
	}


////////////////////////////////////////////////////////////////////////////////
// Load from current position of already-open file
////////////////////////////////////////////////////////////////////////////////
template <class usertype>
short CLogTab<usertype>::Load(						// Returns 0 if successfull, non-zero otherwise
	RFile* pFile)											// In:  RFile to load from
	{
	short sResult = 0;

	// Free any vars currently associated with this table
	FreeVars();

	// Reset counts
	m_sInVars = 0;
	m_sOutVars = 0;
	m_sTotalVars = 0;
	m_sRows = 0;

	// Read first row, which contains variable names to be associated with each column
	char acText[MaxTextLen];
	char cSave = 0;
	bool bEndOfTable;
	bool bEndOfRow;
	bool bOutput = false;
	do	{
		// Get next entry
		sResult = ReadEntry(pFile, acText, sizeof(acText), &bEndOfTable, &bEndOfRow, &cSave);
		if (sResult == 0)
			{
			// Output separator is special case -- it separates output vars from input vars
			if (rspStricmp(acText, LOGTAB_OUTPUT_SEPARATOR) == 0)
				{
				// All subsequent columns are output vars
				bOutput = true;
				}
			else if (rspStricmp(acText, LOGTAB_COMMENT_TOKEN) == 0)
				{
				// If we get a comment token, ignore everything that follows until the end of the row
				while ((sResult == 0) && !bEndOfRow)
					sResult = ReadEntry(pFile, acText, sizeof(acText), &bEndOfTable, &bEndOfRow, &cSave);
				bEndOfRow = false;
				}
			else
				{
				// Make sure total vars doesn't exceed max allowable
				if (m_sTotalVars < MaxVars)
					{
					// Find the var with the specified name
					CLogTabVar<usertype>* pVar;
					if (CLogTabVar<usertype>::FindVar(acText, &pVar) == 0)
						{
						// If this is an output, it must settable or it's an error
						if (bOutput && !pVar->m_bSettable)
							{
							sResult = LOGTAB_ERR_NOT_OUTPUT_VAR;
							TRACE("CLogTab::Load(): '%s' cannot be used as an output!\n", pVar->m_pszName);
							rspMsgBox(LOGTAB_MSG, "'%s' cannot be used as an output.", pVar->m_pszName);
							}
						if (sResult == 0)
							{
							// Add var to array and increment total number of vars
							m_apVars[m_sTotalVars++] = pVar;

							// Increment var's reference count since we're using it
							pVar->IncRefCount();

							// Adjust count of input or output vars, as appropriate
							if (bOutput)
								m_sOutVars++;
							else
								m_sInVars++;
							}
						}
					else
						{
						sResult = LOGTAB_ERR_VAR_NOT_FOUND;
						TRACE("CLogTab::Load(): Cannot resolve variable named '%s'!\n", acText);
						rspMsgBox(LOGTAB_MSG, "Variable named '%s' does not exist or is defined multiple times.  Either way, you are hosed.", acText);
						}
					}
				else
					{
					sResult = LOGTAB_ERR_TOO_MANY_VARS;
					TRACE("CLogTab::Load(): Table exceeds maximum number of vars, which is currently set to %hd!\n", (short)MaxVars);
					rspMsgBox(LOGTAB_MSG, "Table exceeds maximum number of vars, which is currently limited to %hd.", (short)MaxVars);
					}
				}
			}
		} while ((sResult == 0) && !bEndOfRow);

	if (sResult == 0)
		{
		// Make sure we didn't hit the end of the table
		if (bEndOfTable)
			{
			sResult = LOGTAB_ERR_NO_ROWS_AFTER_HEADINGS;
			TRACE("CLogTab::Load(): Unexpected end of table following initial row of column headings!\n");
			rspMsgBox(LOGTAB_MSG, "Unexpected end of table following initial row of column headings.");
			}
		else
			{
			// Make sure table contains at least one input column
			if (m_sInVars < 1)
				{
				sResult = LOGTAB_ERR_NO_INPUTS;
				TRACE("CLogTab::Load(): No inputs were found in initial row of column headings!\n");
				rspMsgBox(LOGTAB_MSG, "No inputs were found in initial row of column headings.");
				}

			// Make sure table contains output separator and at least one output column
			if (!bOutput)
				{
				sResult = LOGTAB_ERR_NO_OUTPUT_SEPARATOR;
				TRACE("CLogTab::Load(): Output separator '%s' was not found in initial row of column headings!\n", LOGTAB_OUTPUT_SEPARATOR);
				rspMsgBox(LOGTAB_MSG, "Output separator '%s' was not found in initial row of column headings.", LOGTAB_OUTPUT_SEPARATOR);
				}
			else if (m_sOutVars < 1)
				{
				sResult = LOGTAB_ERR_NO_OUTPUTS;
				TRACE("CLogTab::Load(): No outputs were found in initial row of column headings!\n");
				rspMsgBox(LOGTAB_MSG, "No outputs were found in initial row of column headings.");
				}
			}

		if (sResult == 0)
			{

			// Read the remaining rows
			do {
				// Each row must contain the same number of columns as the total number of vars
				short sColumn = 0;
				do	{
					// Get next entry
					sResult = ReadEntry(pFile, acText, sizeof(acText), &bEndOfTable, &bEndOfRow, &cSave);
					if (sResult == 0)
						{
						// Check for comment token
						if (rspStricmp(acText, LOGTAB_COMMENT_TOKEN) == 0)
							{
							// If we get a comment token, ignore everything that follows until the end of the row
							while ((sResult == 0) && !bEndOfRow)
								sResult = ReadEntry(pFile, acText, sizeof(acText), &bEndOfTable, &bEndOfRow, &cSave);
							bEndOfRow = false;
							}
						else
							{
							// Make sure number of columns doesn't exceed number of vars
							if (sColumn < m_sTotalVars)
								{
								// Leading character determines type of operand
								short sOffset = 0;
								switch(acText[0])
									{
									case '=':
										sOffset = 1;
									default:	// If there is no special prefix, then we assume "="
										m_cellTable[m_sRows][sColumn].operand = Equal;
										break;

									case '!':
										sOffset = 1;
										m_cellTable[m_sRows][sColumn].operand = Not;
										break;

									case '<':
										sOffset = 1;
										m_cellTable[m_sRows][sColumn].operand = Less;
										break;

									case '>':
										sOffset = 1;
										m_cellTable[m_sRows][sColumn].operand = Greater;
										break;

									case '*':
										sOffset = 1;
										m_cellTable[m_sRows][sColumn].operand = DontCare;
										break;
									}
								if (m_cellTable[m_sRows][sColumn].operand != DontCare)
									{
									// Use this column's var to convert text into value
									short sVal;
									CLogTabVar<usertype>* pVar = m_apVars[sColumn];
									sResult = pVar->TextToVal(&(acText[sOffset]), &sVal);
									if (sResult == 0)
										m_cellTable[m_sRows][sColumn].sEntry = sVal;
									else
										{
										// sResult was properly set by TextToVal()
										TRACE("CLogTab::Load(): '%s' is an invalid setting for '%s'!\n", acText, pVar->m_pszName);
										rspMsgBox(LOGTAB_MSG, "'%s' is an invalid setting for '%s'.", acText, pVar->m_pszName);
										}
									}

								// Increment column count
								if (sResult == 0)
									sColumn++;
								}
							else
								{
								sResult = LOGTAB_ERR_EXTRA_COLUMNS;
								TRACE("CLogTab::Load(): Row #%hd contains extra columns!\n", (short)(m_sRows + 2));
								rspMsgBox(LOGTAB_MSG, "Row #%hd has more columns than the first row.", (short)(m_sRows + 2));
								}
							}
						}
					} while ((sResult == 0) && !bEndOfRow);

				// Make sure there weren't too few columns
				if ((sResult == 0) && (sColumn < m_sTotalVars))
					{
					sResult = LOGTAB_ERR_TOO_FEW_COLUMNS;
					TRACE("CLogTab::Load(): Row #%hd does not contain enough columns!\n", (short)(m_sRows + 2));
					rspMsgBox(LOGTAB_MSG, "Row #%hd contains less columns than the first row.\n", (short)(m_sRows + 2));
					}

				// Increment row count
				if (sResult == 0)
					{
					m_sRows++;

					// If we're at the max number of rows but NOT at the end of the table,
					// then the table obviously contains too many rows.
					if ((m_sRows == MaxRows) && !bEndOfTable)
						{
						sResult = LOGTAB_ERR_TOO_MANY_ROWS;
						TRACE("CLogTab::Load(): Table exceeds maximum number of rows, which is currently set to %hd\n", (short)MaxRows);
						rspMsgBox(LOGTAB_MSG, "Table exceeds maximum number of rows, which is currently limited to %hd.", (short)MaxRows);
						}
					}

				} while ((sResult == 0) && !bEndOfTable);

			// Make sure there was at least one row
			if ((sResult == 0) && (m_sRows < 1))
				{
				sResult = LOGTAB_ERR_NO_ROWS_AFTER_HEADINGS;
				TRACE("CLogTab::Load(): Table has no rows following initial column headings!\n");
				rspMsgBox(LOGTAB_MSG, "Table has no rows following initial column headings.\n");
				}
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Read next table entry.
//
// A table consists of one or more rows, each of which contains one or more
// entry.
//
// The rows in a table are delimited by carriage returns.  The last row is an
// exception, as it may be terminated by the EOF.
//
// The entries within a row are delimited by whitespace.  The last entry in a
// row is an exception, as it may be terminated by the carriage return that
// ends the row or by the EOF.
//
// After reading an entry, this function continues reading beyond it in order
// to determine whether the entry was the last one in the row.  If so,
// pbEndOfRow is set to true.  Otherwise, it is set to false.
//
// Likewise, if it is determined that the entry was the last one in the table,
// then pbEndOfTable is set to true.  Otherwise, it is set to false.
//
// Note that if pbEndOfTable is true, then pbEndOfRow will also be true.
//
// Reaching the end of the table is not considered an error condition.
// However, once this function indicates that the end of the table has been
// reached, then any subsequent calls to this function will return an error
// because no additional entries are available.
//
// Note: Since this function "looks ahead" to determine what follows an entry,
// it may read the first character of the next entry.  If this happens, it will
// save this character in the location pointed at by pcSave, and use that
// character the next time this function is called.  The location pointed at
// by pcSave MUST be set to 0 before the first time this function is called.
// After that, just leave it alone and it will take care of itself.
////////////////////////////////////////////////////////////////////////////////
template <class usertype>
short CLogTab<usertype>::ReadEntry(					// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  RFile to read from
	char* pszEntry,										// Out: Entry is returned in here
	short sMaxEntrySize,									// In:  Maximum size of entry
	bool* pbEndOfTable,									// Out: true if this is last entry in table
	bool* pbEndOfRow,										// Out: true if this is last entry in row
	char* pcSave)											// I/O: Save char from last call (or 0 if none)
	{
	short sResult = 0;

	// If the saved char is non-zero, then we use it as the first character.
	// Otherwise, we need to read a new character from the file.  We then
	// clear the saved char to 0 for next time.
	char ch = *pcSave;
	if (ch == 0)
		pFile->Read(&ch, 1);
	*pcSave = 0;

	// Keep processing chars until we're done.  Being done basically entails
	// throwing out any leading whitespace characters, then adding all subsequent
	// non-whitespace characters to the entry string, and then going through
	// all following characters until we hit another non-whitespace character
	// or the EOF.  If we hit a CR along the way, we set the end-of-row flag.
	// If we hit the EOF, we set the end-of-table flag.  It's kind of tricky,
	// but remember that the idea is that we want to know what follows the
	// current entry, so we need to go beyond it, which usually means going
	// up to the next entry.
	*pbEndOfTable = false;			// Not at end of table
	*pbEndOfRow = false;				// Not at end of row
	short sEntryIndex = 0;			// Start index at 0
	bool bFillingEntry = false;	// Not filling entry yet
	do	{
		// Check for EOF
		if (pFile->IsEOF())
			{
			// If we've added something to the entry, then EOF is okay
			if (sEntryIndex > 0)
				{
				// Set flags for end of row and end of table
				*pbEndOfRow = true;
				*pbEndOfTable = true;
				}
			else
				{
				sResult = LOGTAB_ERR_UNEXPECTED_EOF;
				TRACE("CLogTab::ReadEntry(): Unexpected end of file!\n");
				rspMsgBox(LOGTAB_MSG, "Unexpected end of file.");
				}
			break;
			}

		// Check for file error
		if (pFile->Error())
			{
			sResult = LOGTAB_ERR_READ_ERROR;
			TRACE("CLogTab::ReadEntry(): Error reading from file!\n");
			rspMsgBox(LOGTAB_MSG, "Error reading from file.");
			break;
			}

		// Check if char is whitespace or not
		if (isspace(ch))
			{
			// If we haven't started filling entry yet, then flag is already clear.
			// If we have started, then whitespace ends it.
			bFillingEntry = false;

			// CR is special-case whitespace.  If we've already added to the entry,
			// then CR indicates end of row.  Otherwise, it's just an empty row before
			// the entry, so we ignore it.
			if ((ch == '\r') && (sEntryIndex > 0))
				*pbEndOfRow = true;
			}
		else
			{
			// If we haven't added anything to entry, then start entry now
			if (sEntryIndex == 0)
				bFillingEntry = true;

			// If we're filling the entry, add this new char to it
			if (bFillingEntry)
				{
				// Make sure we don't exceed max entry size (leave room for null!)
				if (sEntryIndex < (sMaxEntrySize - 1))
					{
					// Add to entry, increment index, and tack on a null
					pszEntry[sEntryIndex++] = ch;
					pszEntry[sEntryIndex] = 0;
					}
				else
					{
					sResult = LOGTAB_ERR_TEXT_TOO_LONG;
					TRACE("CLogTab::ReadEntry(): Text too long!  Text so far: '%s'\n", pszEntry);
					rspMsgBox(LOGTAB_MSG, "Text entry is too long.  The text read so far is '%s'.", pszEntry);
					break;
					}
				}
			else
				{
				// This char belongs to the next entry, so save it for next time.
				*pcSave = ch;
				break;
				}
			}

		// Read next char
		pFile->Read(&ch, 1);

		} while (sResult == 0);

	return sResult;
	}


#endif //LOGTAB_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
