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
// prefs.h
//
//		12/11/96	JPW	Implemented reading and writing of ini files,
//							functionallity to get/set variables, and delete/create
//							sections and variables.
//		12/16/96	JPW	Fixed so it will work with the STL stuff that comes with 
//							MSVC 4.1 or newer.  Also fixed a few psz parameters that 
//							should have been const's.
//
//		03/28/97	JMI	Fixed so this'll work with MSVC 4.2.
//		
//		06/29/97 MJR	Replaced STL vector with an RSP list.  STL is an evil
//							entity that should be banished from the face of the earth.
//							Whoever suggested we use it should be shot.  (Good thing
//							I'm the president -- it's against the rules to shoot me.)
//
//							Also moved the ranges for RSPiX types into xxxsystem.h.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef PREFS_H
#define PREFS_H

#include "Blue.h"
#include "prefline.h"


class RPrefs
	{
	public:
		typedef enum ePrefs
			{
			MaxStrLen = 512
			};

	private:
		FILE*						m_pFile;				// Currently open file, or NULL if none
		short						m_sReadOnly;		// Read-only flag (1 = read-only, 0 = not)
		short						m_sErrorStatus;	// Error status (non-zero = error, 0 = not)
		short						m_sModified;		//	Indicates if ini in memory has been modified
		short						m_sDidRead;			// Indicates if ini file has been read yet
		short						m_sUseCRLF;			// Indicates if ini file is using CR/LF pairs
		char						*m_pszFileName;	// Name of file
		char						*m_pszFileMode;	// Mode file was opened in
		RPrefsLineList			m_pllLines;			// List of lines read from ini file.

		// Get iterator to a variable 
		short	GetIteratorToVariable(				// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,					// In:  Section name (without brackets)
			const char* pszVariable,				// In:  Variable name
			RPrefsLineList::Pointer* i);			// Out: iterator to line in list

		// Get iterator to a section
		short	GetIteratorToSection(				// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,					// In:  Section name (without brackets)
			RPrefsLineList::Pointer* i);			// Out: iterator to line in list

	public:
		// Default (and only) constructor
		RPrefs(void);

		// Destructor
		~RPrefs();

		// Open preference file.
		//
		// Two overloaded versions of this function exist.  The first allows the mode
		// to be specified using the same values as used by fopen().  The second
		// attempts to open the file in read/write mode.  If this fails, it attempts
		// to open the file in read-only mode.  If this fails, too, it attempts to
		// create an empty file.
		short Open(								// Returns 0 if successfull, non-zero otherwise
			const char* pszFile,				// In:  Name of preference file
			const char* pszMode);			// In:  Open mode (same as fopen())

		short Open(								// Returns 0 if successfull, non-zero otherwise
			const char* pszFile);			// In:  Name of preference file

		// Read ini file into list of lines.
		short Read();

		// Write ini file back out to disk.
		short Write();

		// Close current preference file
		short Close();

		// Check if any I/O errors have occurred.  This includes the most recent Open()
		// and any I/O errors since then.  Once an error occurs, the only way to clear
		// it is via a successfull Open().
		short IsError(void)					// Returns non-zero if error occurred, 0 otherwise
			{ return m_sErrorStatus; }

		// Check if file is marked as "read only".  If so, then the file cannot be
		// modified or written to.  If this is called at a time when there is no open
		// file, then the return value will be 0.
		short IsReadOnly(void)				// Returns non-zero if read-only, 0 otherwise
			{ return m_sReadOnly; }

		// Check if copy of file in memory has been modified
		short IsModified()
			{ return m_sModified; }

		// Unset modified status
		void SetNotModified()
			{ m_sModified = 0; }

		// Delete the specified variable in the specified section
		short DeleteVariable(					// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable);		// In:  Variable name

		// Delete the specified section
		short DeleteSection(					// Returns 0 if successfull, non-zero otherwise
			const char* pszSection);		// In:  Section name (without brackets)

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
		short SetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			const char* pszValue);			// In:  Value

		short SetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			S8 s8Value);						// In:  Value

		short SetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			U8 u8Value);						// In:  Value

		short SetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			S16 s16Value);						// In:  Value

		short SetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			U16 u16Value);						// In:  Value

		short SetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			S32 s32Value);						// In:  Value

		short SetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			U32 u32Value);						// In:  Value

		short SetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			float fValue);						// In:  Value

		short SetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			double dValue);					// In:  Value

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
		// If an I/O error occurs, then the function returns a non-zero, negative
		// value to indicate the error.
		short GetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			const char* pszDefault,			// In:  Default value
			char* pszValue);					// Out: Value returned here

		short GetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			S8 s8Default,						// In:  Default value
			S8* s8Value);						// Out: Value returned here

		short GetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			U8 u8Default,						// In:  Default value
			U8* u8Value);						// Out: Value returned here

		short GetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			S16 s16Default,					// In:  Default value
			S16* s16Value);					// Out: Value returned here

		short GetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			U16 u16Default,					// In:  Default value
			U16* u16Value);					// Out: Value returned here

		short GetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			S32 s32Default,					// In:  Default value
			S32* s32Value);					// Out: Value returned here

		short GetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			U32 u32Default,					// In:  Default value
			U32* u32Value);					// Out: Value returned here

		short GetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			float fDefault,					// In:  Default value
			float* fValue);					// Out: Value returned here

		short GetVal(							// Returns 0 if successfull, non-zero otherwise
			const char* pszSection,			// In:  Section name (without brackets)
			const char* pszVariable,		// In:  Variable name
			double dDefault,					// In:  Default value
			double* dValue);					// Out: Value returned here

		short Print();							// Used for error checking
	};


#endif //PREFS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
