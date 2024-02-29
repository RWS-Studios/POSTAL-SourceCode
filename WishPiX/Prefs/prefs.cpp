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
// prefs.cpp
// Project: Soon-to-be-RSPiX
//
// This module deals with setting up the system for the game.
//
// History:
//		11/19/96	MJR	Started.
//		12/11/96	JPW	Implemented reading and writing of ini files,
//							functionallity to get/set variables, and delete/create
//							sections and variables.
//		12/16/96	JPW	Fixed so it will work with the STL stuff that comes with 
//							MSVC 4.1 or newer.  Also fixed a few psz parameters that 
//							should have been const's.
//		01/03/97 JPW	Added Read() to functions that access ini file in memory
//							to insure the file is always read once before getting
//							and setting of ini file vars/sections 
//
//		03/28/97	JMI	Fixed so this'll work with MSVC 4.2.
//
//		03/31/97	JMI	Changed sprintf %ud format specifiers (or something) to
//							%u.
//
//		04/21/97 MJR	Changed inclusion of ez_stl from <ez_stl> to "ez_stl",
//							which goes together with adding ez_stl to sourcesafe
//							so there's a local copy in this directory.
//
//		05/08/97	JMI	Added conditions for compiler versions' STL
//							differences (namely "list" vs. "list.h").
//							Also, changed #include <rspix.h> to #include "RSPiX.h".
//
//		06/11/97	JMI	Commented TRACE in GetIteratorToVariable() when variable
//							cannot be found and in GetIteratorToSection() when section
//							cannot be found.
//
//		06/29/97 MJR	Replaced STL vector with an RSP list.  STL is an evil
//							entity that should be banished from the face of the earth.
//							Whoever suggested we use it should be shot.  (Good thing
//							I'm the president -- it's against the rules to shoot me.)
//
//		07/05/97 MJR	Fixed bug in printf() specifications.  Apparently, "%lf"
//							is not a valid spec.  "%f" works fine by itself.
//
//		07/09/97 MJR	Changed so Close() no longer calls Write().
//							Added full support for CR or CR/LF files.
//
//		07/09/97 MJR	Performed an overall cleanup regarding how the error flag
//							gets cleared and set.  Also cleaned up how files are
//							written by close().  Also removed usage of "t" for fopen()
//							modes because it isn't ANSI standard.  Finally, fixed
//							the CR/LF stuff so it actually works.
//
//		08/08/97 MJR	Changed from using ANSI library tmpnam() to our own
//							version that can work in a specific directory.
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <assert.h>
#include <errno.h>

#include "Blue.h"
#include "CYAN/cyan.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/str/str.h"
#else
	#include "str.h"
#endif

#include "prefline.h"
#include "prefs.h"


////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Default (and only) constructor
//
////////////////////////////////////////////////////////////////////////////////
RPrefs::RPrefs(void)
	{
	m_pFile			= NULL;
	m_sReadOnly		= 0;
	m_sModified		= 0;
	m_sDidRead		= 0;
	m_sUseCRLF		= 0;
	m_pszFileName	= 0;
	m_pszFileMode  = 0;
	m_sErrorStatus = 0;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Destructor
//
////////////////////////////////////////////////////////////////////////////////
RPrefs::~RPrefs()
	{
	// Close (in case file was left open)
	Close();

	// Free the name and mode
	delete []m_pszFileName;
	m_pszFileName = NULL;
	delete []m_pszFileMode;
	m_pszFileMode = NULL;

	// Delete all lines and get rid of list nodes
	while (m_pllLines.GetHead())
		{
		delete m_pllLines.GetHeadData();
		m_pllLines.RemoveHead();
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Open preference file.
//
// Two overloaded versions of this function exist.  The first allows the mode
// to be specified using the same values as used by fopen().  The second
// attempts to open the file in read/write mode.  If this fails, it attempts
// to open the file in read-only mode.  If this fails, too, it attempts to
// create an empty file.
//
////////////////////////////////////////////////////////////////////////////////
short RPrefs::Open(					// Returns 0 if successfull, non-zero otherwise
	const char* pszFile)						// In:  Name of preference file
	{
	// Attempt to open in read/write mode
	if (Open(pszFile, "r+") != 0)
		{
		// Attempt to open in read-only mode (file may be read-only)
		if (Open(pszFile, "r") != 0)
			{
			// Attempt to create file
			Open(pszFile, "w+");
			}
		}

	return m_sErrorStatus;
	}

extern const char *FindCorrectFile(const char *pszName, const char *mode);

////////////////////////////////////////////////////////////////////////////////
short RPrefs::Open(					// Returns 0 if successfull, non-zero otherwise
	const char* pszFile,						// In:  Name of preference file
	const char* pszMode)						// In:  Mode (same as fopen())
	{
	ASSERT(pszFile);
	ASSERT(pszMode);

	// Close (in case a file was already open)
	Close();

	// If mode contains "a" (append), "w" (write), or "+" (read and write)
	// then the file won't be read-only.  Otherwise, it will be.
	if (strcspn(pszMode, "aAwW+") < strlen(pszMode))
		m_sReadOnly = 0;
	else
		m_sReadOnly = 1;

	// Attempt to open file
	m_pFile = fopen(FindCorrectFile(pszFile, pszMode), pszMode);
	if (m_pFile != NULL)
		{
		// Make a copy of the file name
		delete []m_pszFileName;
		m_pszFileName = new char[strlen(pszFile) + 1];
		ASSERT(m_pszFileName);
		strcpy(m_pszFileName, pszFile);

		// Make a copy of the file mode
		delete []m_pszFileMode;
		m_pszFileMode = new char[strlen(pszMode) + 1];
		ASSERT(m_pszFileMode);
		strcpy(m_pszFileMode, pszMode);

		// Clear error status
		m_sErrorStatus = 0;
		}
	else
		{
		TRACE("RPrefs::Open(): %s\n",strerror(errno));
		m_sErrorStatus = -1;
		}

	return m_sErrorStatus;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Read and parse ini file into list of lines
//
// If an I/O error occurs, the function returns a negative value.
//
////////////////////////////////////////////////////////////////////////////////
short RPrefs::Read()		// Returns 0 if successfull, non-zero otherwise
	{
	if (!m_sErrorStatus)
		{
		if (!m_sDidRead)
			{
			if (m_pFile != NULL)
				{
				// Seek to start of file
				if (fseek(m_pFile, SEEK_SET, 0) == 0)
					{
					// process lines until EOF is reached or an error occurrs
					//
					// In files written by DOS/Windows, lines are terminated by a CR/LF pair,
					// which is "\n" followed by "\r".  In files written by Mac OS, lines are
					// terminated by a CR ("\n") only.  Under Win32, the standard library
					// functions convert CR/LF into a single CR, but this is NOT done by
					// the Mac versions of these libraries.  On the Mac, fgets() will return
					// a line terminated by a "\n".  The next fgets() will return the "\r"
					// from the previous line along with whatever follows, up to the next
					// "\n".  Therefore, on the Mac, we check if the line begins with a "\r",
					// and if so, we set a flag and then skip over the "\r".  The flag tells
					// us that when we write the file back out, we should use CR/LF pairs to
					// preserve the file's original format.
					char pszLine[RPrefs::MaxStrLen+1];
					RPrefsLine *plTemp;
					while (fgets(pszLine, RPrefs::MaxStrLen, m_pFile) != NULL)
						{
						// Check for "/r".  This is only required by Mac code, but it can't
						// hurt DOS/Windows code, where the fgets() should have converted the
						// "/n/r" into a "/n".
						char* pszFixedLine = pszLine;
						if (*pszFixedLine == '\r')
							{
							// Set flag so we'll know to write file out with CR/LF instead of just CR
							m_sUseCRLF = 1;

							// Skip over the "/r"
							pszFixedLine++;
							
							// If there's nothing after the "\r" it means the previous line
							// was really the last line of the file and this "\r" is just a
							// meaningless, left-over, piece of crap that should be ignored.
							if (*pszFixedLine == '\0')
								continue;
							}

						// Remove newline (if any) from end of line
						short len = strlen(pszFixedLine);
						if (pszFixedLine[len - 1] == '\n')
							pszFixedLine[len - 1] = '\0';

						// Find first non-space char
						char* pszToken = pszFixedLine;
						while ((*pszToken != '\0') && isspace(*pszToken))
							pszToken++;

						// Process line based on whether it's a comment/blank, section, or variable
						switch (*pszToken)
							{
							case '\0':	// Empty line counts as a comment line
							case ';':	// Create comment line
								plTemp = new RPrefsLine(RPrefsLine::Comment, pszFixedLine);
								ASSERT(plTemp);
								break;
							case '[':	// Create section line
								plTemp = new RPrefsLine(RPrefsLine::Section, pszFixedLine);
								ASSERT(plTemp);
								break;
							default:		// Default must be a variable
								plTemp = new RPrefsLine(RPrefsLine::Variable, pszFixedLine);
								ASSERT(plTemp);
								break;
							}
						// Add line to list of lines
						m_pllLines.InsertTail(plTemp);
						} // end while

					if (ferror(m_pFile) == 0)
						{
						// Set flag indicating read was successfully performed
						m_sDidRead = 1;
						}
					else
						{
						m_sErrorStatus = -3;
						TRACE("RPrefs::Read(): fgets(): %s\n", strerror(errno));
						}
					}
				else
					{
					m_sErrorStatus = -2;
					TRACE("RPrefs::Read(): fseek(): %s\n",strerror(errno));
					}
				}
			else
				{
				m_sErrorStatus = -1;
				TRACE("RPrefs::Read(): File not open!\n");
				}
			}
		}
	
	return m_sErrorStatus;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Write current preference file
//
////////////////////////////////////////////////////////////////////////////////
short RPrefs::Write()
	{
	if (!m_sErrorStatus)
		{
		// Make sure we can and should do a write
		if (!m_sReadOnly && m_sModified)
			{
			if (m_pFile != NULL)
				{
				// Read file in case it wasn't already read
				if(Read() == 0)
					{
					// Close file before it gets deleted (and replaced by a new file)
					fclose(m_pFile);
					m_pFile = 0;
					
					// Create a temporary name.  We search backwards from the end
					// of the real filename, looking for the first system separator,
					// or the beginning of the string, whichever comes first.  We
					// then change from that point forward to be "t0000000.tmp",
					// which will work on all systems, even DOS 8.3 file systems.
					// We then check if that filename exists, and if so, we inc the
					// number portion of it and try again until we find a file that
					// doesn't exist, and that becomes our temp file name.
					bool bGotTmp = false;
					char acTmpFileName[RSP_MAX_PATH + 20];
					strcpy(acTmpFileName, m_pszFileName);
					char* pTmp = strrchr(acTmpFileName, RSP_SYSTEM_PATH_SEPARATOR);
					pTmp = (pTmp != NULL) ? pTmp + 1 : acTmpFileName;
					for (long lCount = 0; !bGotTmp && (lCount < 9999999L); lCount++)
						{
						sprintf(pTmp, "t%0.7ld.tmp", (long)lCount);
						FILE* fpTmp = fopen(FindCorrectFile(acTmpFileName, "r"), "r");
						if (fpTmp != NULL)
							fclose(fpTmp);
						else
							bGotTmp = true;
						}
					if (bGotTmp)
						{
						// Create temp file that will contain new ini stuff
						FILE *pfileTmp = fopen(FindCorrectFile(acTmpFileName, "w"), "w");
						if (pfileTmp != NULL)
							{
							// Write lines out to temp file
							for (RPrefsLineList::Pointer i = m_pllLines.GetHead(); i != 0; i = m_pllLines.GetNext(i))
								{
								int res = fprintf(pfileTmp, "%s\n", m_pllLines.GetData(i)->GetLine());
								if ((res >= 0) && m_sUseCRLF)
									res = fprintf(pfileTmp, "\r");
								if(res < 0)
									{
									TRACE("RPrefs::Write(): fprintf() data to temp file: %s\n", strerror(errno));
									m_sErrorStatus = -2;
									break;
									}
								}
							// Close temp file
							fclose(pfileTmp);
							
							if (m_sErrorStatus == 0)
								{
								// Remove old file
								if (remove(FindCorrectFile(m_pszFileName, "w")) == 0)
									{
									// Rename temp file to whatever old file was named
                                    // rename() isn't reliably across filesystems.  --ryan.
									//if (rename(FindCorrectFile(acTmpFileName, "w"), FindCorrectFile(m_pszFileName, "w")) == 0)
    									FILE *in = fopen(FindCorrectFile(acTmpFileName, "w"), "r");
	    								FILE *out = fopen(FindCorrectFile(m_pszFileName, "w"), "w");
                                        if (in && out)
                                        {
                                            while (1)
                                            {
                                                int ch = fgetc(in);
                                                if (ch == EOF) break;
                                                fputc(ch, out);
                                            }
                                        }
                                        if (in) fclose(in);
                                        if (out) fclose(out);
                                        remove(FindCorrectFile(acTmpFileName, "w"));

                                        if (in && out)
										{
										// Open the new file using the original mode
										m_pFile = fopen(FindCorrectFile(m_pszFileName, m_pszFileMode), m_pszFileMode);
										if (m_pFile != NULL)
											{
											// Set flag to indicate ini file in memory is in sync with disk
											m_sModified = 0;
											}
										else
											{
											TRACE("RPrefs::Write(): fopen() of new file: %s\n", strerror(errno));
											m_sErrorStatus = -1;
											}
										}
									else
										{
										TRACE("RPrefs::Write(): rename() of temp file to original file: %s\n", strerror(errno));
										m_sErrorStatus = -4;
										}
									}
								else
									{
									TRACE("RPrefs::Write(): remove() of old ini file: %s\n", strerror(errno));
									m_sErrorStatus = -3;
									}
								}
							}
						else
							{
							TRACE("RPrefs::Write(): fopen() of temp file: %s\n", strerror(errno));
							m_sErrorStatus = -1;
							}
						}
					else
						{
						TRACE("RPrefs::Write(): Couldn't get temp file name!\n");
						m_sErrorStatus = -5;
						}
					}
				else
					{
					TRACE("RPrefs::Write(): Couldn't Read() file!\n");
					}
				}
			else
				{
				m_sErrorStatus = -1;
				TRACE("RPrefs::Write(): File not open!\n");
				}
			}
		}

	return m_sErrorStatus;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Close current preference file
//
////////////////////////////////////////////////////////////////////////////////
short RPrefs::Close()
	{
	if (m_pFile != NULL)
		{
		// If data was read and was modified and files is NOT read only then write it now!
		if (m_sDidRead && !m_sReadOnly && m_sModified)
			Write();

		// Close the file.  We need to check if the file is still valid because
		// something might have gone wrong in Write().
		if (m_pFile != NULL)
			{
			fclose(m_pFile);
			m_pFile = NULL;
			}
		}

	return m_sErrorStatus;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Delete the specified entry in the specified section
//
////////////////////////////////////////////////////////////////////////////////
short RPrefs::DeleteVariable(			// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable)		// In:  Variable name
	{
	ASSERT(pszSection);
	ASSERT(pszVariable);
	
	if (!m_sErrorStatus)
		{
		// Make sure there's a copy in memory
		if(Read() == 0)
			{
			// Get iterator to variable
			RPrefsLineList::Pointer i;
			if (GetIteratorToVariable(pszSection, pszVariable, &i) != 0)
				{
				TRACE("RPrefs::DeleteVariable():GetIteratorToVariable() "
						"unable to find variable.\n");
				m_sErrorStatus = 1;
				}
			else
				{
				m_pllLines.Remove(i);
				m_sModified = 1;
				}
			}
		else
			TRACE("RPrefs::DeleteVariable():RPrefs::Read() read failed: %s\n", strerror(errno));
		}

	return m_sErrorStatus;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Delete the specified section
//
////////////////////////////////////////////////////////////////////////////////
short RPrefs::DeleteSection(		// Returns 0 if successfull, non-zero otherwise
		const char* pszSection)		// In:  Section name (without brackets)
	{
	ASSERT(pszSection);

	if (!m_sErrorStatus)
		{
		// Make sure there's a copy in memory
		if(Read() == 0)
			{
			// Get iterator to section
			RPrefsLineList::Pointer i;
			if (GetIteratorToSection(pszSection, &i) != 0)
				{
				TRACE("RPrefs::DeleteSection():GetIteratorToSection() unable to find section");
				m_sErrorStatus = 1;
				}
			else
				{
				// Remove everything from this section name up to next section name
				do	{
					RPrefsLineList::Pointer j = m_pllLines.GetNext(i);
					m_pllLines.Remove(i);
					i = j;
					if (i == 0)
						break;
					} while (m_pllLines.GetData(i)->GetType() != RPrefsLine::Section);
				m_sModified = 1;
				}
			}
		else
			TRACE("RPrefs::DeleteSection():RPrefs::Read() read failed: %s\n", strerror(errno));
		}

	return m_sErrorStatus;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Set specified entry in the specified section to the specified value.
//
// Several overloaded versions of this function exist, each dealing with
// different types, ranging from a string to all the basic integer and
// floating-point types.
//
// If the section does not exist, it will be created.  Likewise, if the entry
// does not exist, it will be created.
//
// If an I/O error occurs, the function returns a non-zero negative value.
//
////////////////////////////////////////////////////////////////////////////////
short RPrefs::SetVal(				// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	const char* pszValue)			// In:  Value
	{
	// Iterator for section and variable in list of lines
	RPrefsLineList::Pointer iSection, iVariable;
	RPrefsLine						*pplTemp;
	char								pszLine[128];

	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(pszValue);

	// Make sure there's a copy in memory
	if(Read() == 0)
		{
		// Determine if section exists by trying to get an iterator to the section
		if (GetIteratorToSection(pszSection, &iSection) == 0)
			{
			// Section exists.  No need to create a new one.
			// Determine if variable exists by trying to get an iterator to the variable
			if (GetIteratorToVariable(pszSection, pszVariable, &iVariable) == 0)
				// Found variable and section.  Must change value of existing variable
				m_pllLines.GetData(iVariable)->SetVariableValue(pszValue);
			else
				{	// Variable not found but section was.  Add a new variable
				sprintf(pszLine, "%s = %s", pszVariable, pszValue);
				pplTemp = new RPrefsLine(RPrefsLine::Variable, pszLine);
				ASSERT(pplTemp);
				m_pllLines.InsertAfter(pplTemp, iSection);
				m_sErrorStatus = 0;
				}
			}
		else
			{	// Section Not found.  Create empty comment line followed by new section and variable
			pplTemp = new RPrefsLine(RPrefsLine::Comment, "");
			ASSERT(pplTemp);
			m_pllLines.InsertTail(pplTemp);
			sprintf(pszLine, "[%s]", pszSection);
			pplTemp = new RPrefsLine(RPrefsLine::Section, pszLine);
			ASSERT(pplTemp);
			m_pllLines.InsertTail(pplTemp);
			sprintf(pszLine, "%s = %s", pszVariable, pszValue);
			pplTemp = new RPrefsLine(RPrefsLine::Variable, pszLine);
			ASSERT(pplTemp);
			m_pllLines.InsertTail(pplTemp);
			m_sErrorStatus = 0;
			}
		m_sModified = 1;
		}
	else
		TRACE("RPrefs::SetVal():RPrefs::Read() read failed: %s\n", strerror(errno));

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::SetVal(				// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	S8 s8Value)							// In:  Value
	{
	char	pszValue[8];

	ASSERT(pszSection);
	ASSERT(pszVariable);
	sprintf(pszValue, "%d", (int) s8Value);
	SetVal(pszSection, pszVariable, pszValue);

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::SetVal(				// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	U8 u8Value)							// In:  Value
	{
	char	pszValue[8];

	ASSERT(pszSection);
	ASSERT(pszVariable);
	sprintf(pszValue, "%u", (unsigned) u8Value);
	SetVal(pszSection, pszVariable, pszValue);

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::SetVal(				// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	S16 s16Value)						// In:  Value
	{
	char	pszValue[8];

	ASSERT(pszSection);
	ASSERT(pszVariable);
	sprintf(pszValue, "%d", (int) s16Value);
	SetVal(pszSection, pszVariable, pszValue);

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::SetVal(				// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	U16 u16Value)						// In:  Value
	{
	char	pszValue[8];

	ASSERT(pszSection);
	ASSERT(pszVariable);
	sprintf(pszValue, "%u", (unsigned) u16Value);
	SetVal(pszSection, pszVariable, pszValue);

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::SetVal(				// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	S32 s32Value)						// In:  Value
	{
	char	pszValue[16];

	ASSERT(pszSection);
	ASSERT(pszVariable);
	sprintf(pszValue, "%ld", (long) s32Value);
	SetVal(pszSection, pszVariable, pszValue);

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::SetVal(				// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	U32 u32Value)						// In:  Value
	{
	char	pszValue[16];

	ASSERT(pszSection);
	ASSERT(pszVariable);
	sprintf(pszValue, "%lu", (unsigned long) u32Value);
	SetVal(pszSection, pszVariable, pszValue);

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::SetVal(				// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	float fValue)						// In:  Value
	{
	char	pszValue[32];

	ASSERT(pszSection);
	ASSERT(pszVariable);
	sprintf(pszValue, "%f", fValue);
	SetVal(pszSection, pszVariable, pszValue);

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::SetVal(				// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	double dValue)						// In:  Value
	{
	char	pszValue[32];

	ASSERT(pszSection);
	ASSERT(pszVariable);
	sprintf(pszValue, "%f", dValue);
	SetVal(pszSection, pszVariable, pszValue);

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
// Get iterator to a section
////////////////////////////////////////////////////////////////////////////////
short	RPrefs::GetIteratorToSection(		// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,					// In:  Section name (without brackets)
	RPrefsLineList::Pointer* pi)			// Out: iterator to line in list
	{
	assert (pszSection);
	assert (pi);

	if (!m_sErrorStatus)
		{
		if (m_pllLines.GetHead() == 0)
			{
			TRACE("RPrefs::GetIteratorToSection():m_pllLines.empty(): list is empty.\n");
			m_sErrorStatus = 1;
			}
		else
			{
			char	pszSectionName[RPrefs::MaxStrLen];

			// Find section
			short sFoundSection = 0;
			for (*pi = m_pllLines.GetHead(); *pi != 0; *pi = m_pllLines.GetNext(*pi))
				{
				// Only looking for sections
				if (m_pllLines.GetData(*pi)->GetType() == RPrefsLine::Section)
					{
					// Get section name
					if (m_pllLines.GetData(*pi)->GetSectionName(pszSectionName) == 0)
						{
						// Is this the section name we are looking for?
						if (rspStricmp(pszSectionName, pszSection) == 0)
							{
							sFoundSection = 1;
							break;
							}
						}
					}
				}

			if (!sFoundSection)	// If we didn't find section, return non-zero
				{
	//			TRACE("RPrefs::GetIteratorToSection(): section not found.\n");
				m_sErrorStatus = 2;
				}
			}
		}

	return m_sErrorStatus;
	}


////////////////////////////////////////////////////////////////////////////////
// Get iterator to a variable 
////////////////////////////////////////////////////////////////////////////////
short	RPrefs::GetIteratorToVariable(	// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,					// In:  Section name (without brackets)
	const char* pszVariable,				// In:  Variable name
	RPrefsLineList::Pointer* pi)			// Out: iterator to line in list
	{
	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(pi);

	if (!m_sErrorStatus)
		{
		if (m_pllLines.GetHead() == 0)
			{
			TRACE("RPrefs::GetIteratorToVariable():m_pllLines.empty(): list is empty.\n");
			m_sErrorStatus = 1;
			}
		else
			{
			char	pszVariableName[RPrefs::MaxStrLen];

			// Get iterator to section
			m_sErrorStatus = GetIteratorToSection(pszSection, pi);
			if (m_sErrorStatus == 0)
				{
				// Search for variable (note that we start on the line after the section name)
				short sFoundVariable = 0;
				for (*pi = m_pllLines.GetNext(*pi); *pi != 0; *pi = m_pllLines.GetNext(*pi))
					{
					// Stop if we reach the next section
					if (m_pllLines.GetData(*pi)->GetType() == RPrefsLine::Section)
						break;
					
					// Only looking for variables
					if (m_pllLines.GetData(*pi)->GetType() == RPrefsLine::Variable)
						{
						// Get variable name
						if (m_pllLines.GetData(*pi)->GetVariableName(pszVariableName) == 0)
							{
							// Is this the variable name we are looking for
							if (rspStricmp(pszVariableName, pszVariable) == 0)
								{
								sFoundVariable = 1;
								break;
								}
							}
						}
					}

				if (!sFoundVariable)	
					{
	//				TRACE("RPrefs::GetIteratorToVariable(): variable not found.\n");
					m_sErrorStatus = 2;
					}
				}
			}
		}

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Get the value associated with the specified entry in the specified section.
//
// Several overloaded versions of this function exist, each dealing with
// different types, ranging from a string to all the basic integer and
// floating-point types.
//
// If the entry does not exist in the file, then the specified default value
// is used.
//
// If the value cannot be converted to the requested type (invalid characters,
// overflow, etc.) then the value is set to 0 and the function returns a non-
// zero, positive value to indicate an error.
//
////////////////////////////////////////////////////////////////////////////////
short RPrefs::GetVal(							// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	const char* pszDefault,			// In:  Default value
	char* pszValue)					// Out: Value returned here
	{
	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(pszDefault);
	ASSERT(pszValue);

	//if (m_sDidRead != 1)
	if (Read() == 0)
		{
		// Get iterator to variable
		RPrefsLineList::Pointer i;
		m_sErrorStatus = GetIteratorToVariable(pszSection, pszVariable, &i);
		// Process variable if it was found
		if (m_sErrorStatus == 0)
			{
			// Get variable's value
			m_sErrorStatus = m_pllLines.GetData(i)->GetVariableValue(pszValue);
			// Return code of 3 indicates an empty value
			if (m_sErrorStatus == 3)
				{
				strcpy(pszValue, pszDefault);	// Go with default
				m_sErrorStatus = 0;
				}
			else 
				if (m_sErrorStatus != 0)	// Any other error code means a syntax error
					m_sErrorStatus = 1;
			}
		else
			{
			if (pszValue != pszDefault) // pointer comparison.
				strcpy(pszValue, pszDefault);	// var not found, go with default
			m_sErrorStatus = 0;				// Not finding a var should not be reported
			}										// as an error on by return
		}
	else
		TRACE("RPrefs::GetVal():RPrefs::Read() read failed: %s\n", strerror(errno));

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::GetVal(							// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	S8 s8Default,						// In:  Default value
	S8* s8Value)						// Out: Value returned here
	{
	char	pszValue[RPrefs::MaxStrLen], pszDefault[RPrefs::MaxStrLen], *pszEndPtr;
	long	lRes;

	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(s8Value);
	
	sprintf(pszDefault, "%d", (int)s8Default);
	m_sErrorStatus = GetVal(pszSection, pszVariable, pszDefault, pszValue);
	if (m_sErrorStatus == 0)
		{
		lRes = strtol(pszValue, &pszEndPtr, 10);
		if (((*pszEndPtr == '\0') || isspace(*pszEndPtr)) && 
				lRes >= S8_MIN && lRes <= S8_MAX)
			*s8Value = (S8) lRes;
		else
			{
			m_sErrorStatus = 1;
			*s8Value = 0;
			}

		}
	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::GetVal(							// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	U8 u8Default,						// In:  Default value
	U8* u8Value)						// Out: Value returned here
	{
	char	pszValue[RPrefs::MaxStrLen], pszDefault[RPrefs::MaxStrLen], *pszEndPtr;
	long	lRes;

	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(u8Value);

	sprintf(pszDefault, "%u", (unsigned)u8Default);
	m_sErrorStatus = GetVal(pszSection, pszVariable, pszDefault, pszValue);
	if (m_sErrorStatus == 0)
		{
		lRes = strtol(pszValue, &pszEndPtr, 10);
		if (((*pszEndPtr == '\0') || isspace(*pszEndPtr)) && lRes >= U8_MIN && lRes <= U8_MAX)
			*u8Value = (U8) lRes;
		else
			{
			m_sErrorStatus = 1;
			*u8Value = 0;
			}
		}
	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::GetVal(							// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	S16 s16Default,					// In:  Default value
	S16* s16Value)						// Out: Value returned here
	{
	char				pszValue[RPrefs::MaxStrLen], pszDefault[RPrefs::MaxStrLen], *pszEndPtr;
	long	lRes;

	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(s16Value);

	sprintf(pszDefault, "%d", (int)s16Default);
	m_sErrorStatus = GetVal(pszSection, pszVariable, pszDefault, pszValue);
	if (m_sErrorStatus == 0)
		{
		lRes = strtol(pszValue, &pszEndPtr, 10);
		if (((*pszEndPtr == '\0') || isspace(*pszEndPtr)) && lRes >= S16_MIN && lRes <= S16_MAX)
			*s16Value = (S16) lRes;
		else
			{
			m_sErrorStatus = 1;
			*s16Value = 0;
			}
		}
	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::GetVal(							// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	U16 u16Default,					// In:  Default value
	U16* u16Value)						// Out: Value returned here
	{
	char	pszValue[RPrefs::MaxStrLen], pszDefault[RPrefs::MaxStrLen], *pszEndPtr;
	long	lRes;

	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(u16Value);

	sprintf(pszDefault, "%u", (unsigned)u16Default);
	m_sErrorStatus = GetVal(pszSection, pszVariable, pszDefault, pszValue);
	if (m_sErrorStatus == 0)
		{
		lRes = strtol(pszValue, &pszEndPtr, 10);
		if (((*pszEndPtr == '\0') || isspace(*pszEndPtr)) 
				&& lRes >= U16_MIN && lRes <= U16_MAX)
			*u16Value = (U16) lRes;
		else
			{
			m_sErrorStatus = 1;
			*u16Value = 0;
			}
		}
	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::GetVal(							// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	S32 s32Default,					// In:  Default value
	S32* s32Value)						// Out: Value returned here
	{
	char		pszValue[RPrefs::MaxStrLen], pszDefault[RPrefs::MaxStrLen], *pszEndPtr;
	double	dRes;

	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(s32Value);

	sprintf(pszDefault, "%ld", (int)s32Default);
	m_sErrorStatus = GetVal(pszSection, pszVariable, pszDefault, pszValue);
	if (m_sErrorStatus == 0)
		{
		dRes = strtod(pszValue, &pszEndPtr);
		if (((*pszEndPtr == '\0') || isspace(*pszEndPtr)) && 
				dRes >= S32_MIN && dRes <= S32_MAX)
			*s32Value = (S32) dRes;
		else
			{
			m_sErrorStatus = 1;
			*s32Value = 0;
			}
		}
	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::GetVal(							// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	U32 u32Default,					// In:  Default value
	U32* u32Value)						// Out: Value returned here
	{
	char		pszValue[RPrefs::MaxStrLen], pszDefault[RPrefs::MaxStrLen], *pszEndPtr;
	double	dRes;

	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(u32Value);

	sprintf(pszDefault, "%lu", (unsigned)u32Default);
	m_sErrorStatus = GetVal(pszSection, pszVariable, pszDefault, pszValue);
	if (m_sErrorStatus == 0)
		{
		dRes = strtod(pszValue, &pszEndPtr);
		if (((*pszEndPtr == '\0') || isspace(*pszEndPtr)) 
				&& dRes >= U32_MIN && dRes <= U32_MAX)
			*u32Value = (U32) dRes;
		else
			{
			m_sErrorStatus = 1;
			*u32Value = 0;
			}
		}
	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::GetVal(							// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	float fDefault,					// In:  Default value
	float* fValue)						// Out: Value returned here
	{
	char		pszValue[RPrefs::MaxStrLen], pszDefault[RPrefs::MaxStrLen], *pszEndPtr;
	double	dRes;

	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(fValue);

	sprintf(pszDefault, "%f", fDefault);
	m_sErrorStatus = GetVal(pszSection, pszVariable, pszDefault, pszValue);
	if (m_sErrorStatus == 0)
		{
		dRes = strtod(pszValue, &pszEndPtr);
		if (((*pszEndPtr == '\0') || isspace(*pszEndPtr)) && dRes >= FLT_MIN && dRes <= FLT_MAX)
			*fValue = (float) dRes;
		else
			{
			m_sErrorStatus = 1;
			*fValue = (float) 0;
			}
		}
	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
short RPrefs::GetVal(							// Returns 0 if successfull, non-zero otherwise
	const char* pszSection,			// In:  Section name (without brackets)
	const char* pszVariable,		// In:  Variable name
	double dDefault,					// In:  Default value
	double* dValue)					// Out: Value returned here
	{
	char		pszValue[RPrefs::MaxStrLen], pszDefault[RPrefs::MaxStrLen], *pszEndPtr;
	double	dRes;

	ASSERT(pszSection);
	ASSERT(pszVariable);
	ASSERT(dValue);

	sprintf(pszDefault, "%f", dDefault);
	m_sErrorStatus = GetVal(pszSection, pszVariable, pszDefault, pszValue);
	if (m_sErrorStatus == 0)
		{
		dRes = strtod(pszValue, &pszEndPtr);
		if ((*pszEndPtr == '\0') || isspace(*pszEndPtr))
			*dValue = dRes;
		else
			{
			m_sErrorStatus = 1;
			*dValue = 0;
			}
		}
	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////

short RPrefs::Print()
	{
	if (Read() == 0)
		{
		if (m_pllLines.GetHead() == 0)
			m_sErrorStatus = 1;
		else
			{
			for (RPrefsLineList::Pointer i = m_pllLines.GetHead(); i != 0; i = m_pllLines.GetNext(i))
				{
				switch (m_pllLines.GetData(i)->GetType())
					{
					case RPrefsLine::Comment:
						printf("[comment]  ");
						break;
					case RPrefsLine::Section:
						printf("[section]  ");
						break;
					case RPrefsLine::Variable:
						printf("[variable] ");
						break;
					default:
						break;
					}
				printf("%s\n", m_pllLines.GetData(i)->GetLine());
				}
			}
		}
	else
		TRACE("RPrefs::Print():RPrefs::Read() read failed: %s\n", strerror(errno));

	return m_sErrorStatus;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
