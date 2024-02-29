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
////////////////////////////////////////////////////////////////////////////////
//
// string.h
// Project: Nostril (aka Postal)
//
// History:
//		(see associated .CPP file)
//
//		09/28/99	JMI	Unified operator< into one global operator that works for
//							RString vs. char*, char* vs. char*, and RString vs. RString.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef RSTRING_H
#define RSTRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/File/file.h"
#else
	#include "file.h"
#endif

class RString
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	protected:
		enum
			{
//			MaxCharLen = 4,		// "-127" (could be 255 if char is unsigned; len would still work)
//			MaxUCharLen = 3,		// "255"
			MaxShortLen = 6,		// "-32768"
			MaxUShortLen = 5,		// "65535"
			MaxLongLen = 11,		// "-2147483648"
			MaxULongLen = 10,		// "4294967295"
			};

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	protected:
		char* m_pBuf;				// Pointer to buffer memory, or ms_pszEmpty if none exists
		long m_lBufSize;			// Size of buffer (0 means there is no buffer!)
		long m_lStrLen;			// Length of string (up to but not including the terminating null)

		// Static pointer that is initialized to point at an empty string.  When
		// there is no buffer, a string's buffer pointer is set equal to this
		// value.  This makes it a bit faster to cast an RStrng as a char*.  Lots
		// of work to go through for a small gain, but I liked the idea!
		static char* ms_pszEmpty;

	//---------------------------------------------------------------------------
	// Private helpers
	//---------------------------------------------------------------------------
	protected:
		// Common constructor code
		void Construct(long lMinimumSize)
			{
			m_pBuf = ms_pszEmpty;
			m_lBufSize = 0;
			m_lStrLen = 0;
			if (lMinimumSize)
				Grow(lMinimumSize);
			}

		// Free buffer if one exists.
		void FreeBuf(void)
			{
			if (m_lBufSize > 0)
				{
				free(m_pBuf);
				m_pBuf = ms_pszEmpty;
				m_lBufSize = 0;
				}
			m_lStrLen = 0;
			}

	//---------------------------------------------------------------------------
	// Destructor
	//---------------------------------------------------------------------------
	public:
		// Destructor
		~RString()
			{
			FreeBuf();
			}

	//---------------------------------------------------------------------------
	// Buffer-oriented functions
	//---------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Get buffer size (this will always be at LEAST one larger than string length)
		////////////////////////////////////////////////////////////////////////////////
		long GetSize(void) const
			{ return m_lBufSize; }

		////////////////////////////////////////////////////////////////////////////////
		// Grow the buffer to the specified size.  If the buffer is already greater than
		// or equal to the specified size, then this will have no effect.  Otherwise,
		// the buffer is grown to the specified size.  The string is unaffected.
		////////////////////////////////////////////////////////////////////////////////
		void Grow(long lMinimumSize);

		////////////////////////////////////////////////////////////////////////////////
		// Shrink the buffer to the specified size.  If the buffer is already less than
		// or equal to the specified size,  then this will have no effect.  Otherwise,
		// the buffer is reduced to the specified size.  If the specified size is 0,
		// the buffer, if any, will be freed.  If the specified size is less than is
		// required to hold the current string (including the terminating null), then
		// the string is truncated to a length one less than the specified size, and a
		// new terminating null is written.  Note that a size of 1 will result in an
		// empty string since there will only be room for the terminating null.
		////////////////////////////////////////////////////////////////////////////////
		void Shrink(long lMaximumSize);

		////////////////////////////////////////////////////////////////////////////////
		// Compact the buffer to the minimum size required to hold the current string.
		// If the string is empty, the buffer is freed.
		////////////////////////////////////////////////////////////////////////////////
		void Compact(void);

	//---------------------------------------------------------------------------
	// String-oriented functions
	//---------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Clear the string to a length of 0
		////////////////////////////////////////////////////////////////////////////////
		void Clear(void)
			{
			// If there is a buffer, truncate string (otherwise, string is already empty)
			if (m_lBufSize > 0)
				{
				*m_pBuf = 0; // put terminating null at start of buffer
				m_lStrLen = 0;
				}
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get string length (does not include the terminating null or any unused buffer
		// space)
		////////////////////////////////////////////////////////////////////////////////
		long GetLen(void) const
			{ return m_lStrLen; }

		////////////////////////////////////////////////////////////////////////////////
		// Get the character at the specified position in the string.  If the position
		// is negative or beyond the end of the string, the returned value will be 0.
		////////////////////////////////////////////////////////////////////////////////
		char GetAt(long lPos) const
			{
			if ((lPos >= 0) && (lPos < m_lStrLen)) // implies m_lStrLen > 0
				return m_pBuf[lPos];
			return 0;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Array operator, works the same as GetAt(), but using array notation.
		////////////////////////////////////////////////////////////////////////////////
		char operator [](long lPos) const
			{ return GetAt(lPos); }

		////////////////////////////////////////////////////////////////////////////////
		// Set the specified character at the specified position in the string.  If the
		// specified character is 0 (null) then the string's length is shortened (the
		// new length will be equal to the specified position).  If the position is
		// negative or beyind the end of the string, this function does nothing.
		////////////////////////////////////////////////////////////////////////////////
		void SetAt(long lPos, char c)
			{
			if ((lPos >= 0) && (lPos < m_lStrLen)) // implies m_lStrLen > 0
				{
				m_pBuf[lPos] = c;
				if (c == 0)
					m_lStrLen = lPos;
				}
			}
			 
		////////////////////////////////////////////////////////////////////////////////
		// Conversion operator, which allows an RString to be converted (or cast) to a
		// char* so it can be used like a standard C-style string.  If the current
		// string doesn't actually have a buffer (which may be the case when the string
		// length is 0), then a pointer to an empty string is returned.  This function
		// can therefore be called at any time and will always return a valid pointer.
		// If the contents of the string is modified in any way through this pointer
		// then Update() must be called before using any other string functions!
		////////////////////////////////////////////////////////////////////////////////
	   operator char*() const
			{ return m_pBuf; }

		////////////////////////////////////////////////////////////////////////////////
		// Update the internal state of the string.  This must be called if the string
		// has been modified via the pointer returned by the char* operator.  It simply
		// updates the internal state of the string to match its current contents.
		////////////////////////////////////////////////////////////////////////////////
		void Update(void)
			{
			long lLen = strlen(m_pBuf);
			if (lLen < m_lBufSize)
				m_lStrLen = lLen;
			else
				{
				TRACE("RString::Update(): Buffer was overwritten!  Memory has been corrupted!\n");
				ASSERT(0); // buffer has been overwritten
				}
			}

		////////////////////////////////////////////////////////////////////////////////
		// Format string using sprintf-like method.  The specified size is used to make
		// sure the buffer is large enough to hold the generated string (the specified
		// size is passed to the Grow() function -- see it for more details).
		//
		// WARNING: The ANSI standard vsprintf function is used to format the string,
		// which is good because the results will be consistant with the rest of the
		// printf family, but is also very bad because there is no way to limit the
		// number of  characters that are actually written to the buffer!  Therefore,
		// this function is fully reliant on the user to ensure the buffer is large
		// enough to hold the resulting string.  This represents a gaping hole in
		// RString's functionality.
		//
		// Note #1: At some point, I would like to modify the source to vsprintf so it
		// works directly with RString and takes advantage of its dynamic sizing.  If
		// that were done, the size parameter could be removed from this function, and
		// other internal RString code that uses sprintf could be simplified as well.
		//
		// Note #2: Microsoft DOES supply a non-ANSI variation called _vsnprintf() that
		// takes a "maximum output size" parameter.  For now, we use this variation
		// whenever we're compiled under Microsoft.
		//
		// Returns number of characters written, or -1 if an error occurred (this is
		// basically the value returned by vsprintf.)
		////////////////////////////////////////////////////////////////////////////////
		long Format(long lMinSize, char* format, ...);

		////////////////////////////////////////////////////////////////////////////////
		// Create a new RString based on this string's first 'lLen' characters.  If
		// lLen is negative, the returned string will be empty.  Otherwise, the returned
		// string's length will be lLen or this string's length, whichever is less.
		////////////////////////////////////////////////////////////////////////////////
		RString Left(long lLen) const;

		////////////////////////////////////////////////////////////////////////////////
		// Create a new RString based on this string's last 'lLen' characters.  If lLen
		// is negative, the returned string will be empty.  Otherwise, the returned
		// string's length will be lLen or this string's length, whichever is less.
		////////////////////////////////////////////////////////////////////////////////
		RString Right(long lLen) const;

		////////////////////////////////////////////////////////////////////////////////
		// Create a new RString based on a portion of this string, starting at lPos and
		// proceeding for lLen characters.  If lPos is negative or beyond the end of
		// this string, or if lLen is negative, the returned string will be empty.  The
		// specified lLen will be clipped if necessary to avoid going past the end
		// of this string.
		////////////////////////////////////////////////////////////////////////////////
		RString Mid(long lPos, long lLen) const;

		////////////////////////////////////////////////////////////////////////////////
		// Create a new RString based on the specified range of characters from this
		// string.  If either position is negative the returned string will be empty.
		// The positions are clipped if they are past the end of this string.  lPos1
		// must be less than or equal to lPos2.
		////////////////////////////////////////////////////////////////////////////////
		RString Range(long lPos1, long lPos2) const;

		////////////////////////////////////////////////////////////////////////////////
		// Insert any of the supported types into the string at the specified position.
		// The current string contents, from the character at the specified position up
		// to the last character, inclusive, are moved forward to make room for the new
		// characters.  The position is handled slightly differently by this function
		// in that it is valid for it to equal the string length, in which case the new
		// characters are basically appended onto the end of the string (in most other
		// functions, the position must be less than the string length).  If the
		// position is negative or greater than the string length, then this function
		// will do nothing.
		////////////////////////////////////////////////////////////////////////////////
		// Note: The Insert() functions were one of the last things to be added, which
		// is too bad, because so many other things, like += and =, could have simply
		// called Insert()!  It would be little less efficient, but much less code to
		// maintain.
		void Insert(long lPos, const RString& str);
		void Insert(long lPos, const char* psz);
		void Insert(long lPos, char c);

		// These would be very cool variations, not very difficult to write, I just
		// didn't have the time to spend right now.
		#if 0
		void Insert(long lPos, const RString& str, long lLen);
		void Insert(long lPos, const RString& str, long lPos, long lLen);
		#endif

		// I took these out because their usage was very clumsy.  The problem is that
		// sprintf() is being used to convert these values into text, but there is no
		// way to predict how large the result would be (well, not exactly no way...).
		// So, I could simply go with the maximum possible size and insert that, but in
		// most cases you'd end up with the number followed by a bunch of spaces, which
		// is, to reiterate, very clumsy.
		#if 0
		void Insert(long lPos, short s);
		void Insert(long lPos, unsigned short us);
		void Insert(long lPos, long l);
		void Insert(long lPos, unsigned long ul);
		#endif

		////////////////////////////////////////////////////////////////////////////////
		// Delete the specified number of characters starting at the specified position.
		// If the position or length are negative, the string is left unmodified.
		////////////////////////////////////////////////////////////////////////////////
		void Delete(long lPos, long lLen);

		////////////////////////////////////////////////////////////////////////////////
		// Convert string to upper case
		////////////////////////////////////////////////////////////////////////////////
		void ToUpper(void);

		////////////////////////////////////////////////////////////////////////////////
		// Convert string to lower case
		////////////////////////////////////////////////////////////////////////////////
		void ToLower(void);

		////////////////////////////////////////////////////////////////////////////////
		// Load a previously saved string from the specified RFile.  Calls Clear() if
		// an error occurs while loading.  Returns 0 if successfull, non-zero otherwise.
		////////////////////////////////////////////////////////////////////////////////
		short Load(RFile* pFile);

		////////////////////////////////////////////////////////////////////////////////
		// Save this string to the specified RFile.  This RString is not modified by the
		// save, even if an error occurs.  Returns 0 if successfull, non-zero otherwise.
		////////////////////////////////////////////////////////////////////////////////
		short Save(RFile* pFile) const;

	//---------------------------------------------------------------------------
	// operator =
	// Assigns a string to an existing RString based on the contents of another
	// RString or other supported type.
	//---------------------------------------------------------------------------
	public:
		// Assign specified RString
		const RString& operator=(const RString& rhs)
			{
			if (rhs.m_lStrLen > 0)
				{
				Grow(rhs.m_lStrLen + 1);  // size is always > 0, so this will always return with a valid buffer
				memcpy(m_pBuf, rhs.m_pBuf, rhs.m_lStrLen + 1); // includes null in copy!
				m_lStrLen = rhs.m_lStrLen;
				}
			else
				Clear();
			return *this;
			}

		// Assign specified C-style string
		const RString& operator=(const char* rhs)
			{
			ASSERT(rhs != NULL);
			if (rhs != NULL)
				{
				long lLen = strlen(rhs);
				Grow(lLen + 1); // size is always > 0, so this will always return with a valid buffer
				memcpy(m_pBuf, rhs, lLen + 1);
				m_lStrLen = lLen;
				}
			else
				Clear();
			return *this;
			}

		// Assign character (treated as ASCII, not as number!)
		const RString& operator=(char rhs)
			{
			Grow(1 + 1); // size is always > 0, so this will always return with a valid buffer
			m_pBuf[0] = rhs;
			m_pBuf[1] = 0;
			m_lStrLen = 1;
			return *this;
			}

		// Assign string representation of specified number
		const RString& operator=(short rhs)
			{
			Grow(MaxShortLen + 1); // size is always > 0, so this will always return with a valid buffer
			m_lStrLen = sprintf(m_pBuf, "%hd", (short)rhs);
			return *this;
			}

		// Assign string representation of specified number
		const RString& operator=(unsigned short rhs)
			{
			Grow(MaxUShortLen + 1); // size is always > 0, so this will always return with a valid buffer
			m_lStrLen = sprintf(m_pBuf, "%hu", (unsigned short)rhs);
			return *this;
			}

		// Assign string representation of specified number
		const RString& operator=(long rhs)
			{
			Grow(MaxLongLen + 1); // size is always > 0, so this will always return with a valid buffer
			m_lStrLen = sprintf(m_pBuf, "%ld", (long)rhs);
			return *this;
			}

		// Assign string representation of specified number
		const RString& operator=(unsigned long rhs)
			{
			Grow(MaxULongLen + 1); // size is always > 0, so this will always return with a valid buffer
			m_lStrLen = sprintf(m_pBuf, "%lu", (unsigned long)rhs);
			return *this;
			}

	//---------------------------------------------------------------------------
	// Constructors
	// Creates a new RString, optionally based on any supported type and also
	// optionally with a minimum buffer size.
	//---------------------------------------------------------------------------
	public:
		// Create empty string (with no buffer to start with)
		RString(void)
			{
			Construct(0);
			}

		// Create empty string with minimum buffer size (see Grow())
		RString(long lMinimumSize)
			{
			Construct(lMinimumSize);
			}

		// Create string based on existing string, optionally with minimum buffer size (see Grow())
		RString(const RString& rhs, long lMinimumSize = 0)
			{
			Construct(lMinimumSize);
			operator=(rhs);
			}

		// Create string based on C-style string, optionally with minimum buffer size (see Grow())
		RString(const char* rhs, long lMinimumSize = 0)
			{
			Construct(lMinimumSize);
			operator=(rhs);
			}

	//---------------------------------------------------------------------------
	// operator +=
	// This operator appends an RString or other supported type onto an existing
	// RString and returns a reference to the revised RString.
	//---------------------------------------------------------------------------
	public:
		// Append specified string
		const RString& operator+=(const RString& rhs)
			{
			Insert(m_lStrLen, rhs);
			return *this;
			}

		// Append specified C-style string
		const RString& operator+=(const char* rhs)
			{
			Insert(m_lStrLen, rhs);
			return *this;
			}

		// Append character (treated as ASCII, not as number!)
		const RString& operator+=(char rhs)
			{
			Insert(m_lStrLen, rhs);
			return *this;
			}

		// Append string representation of specified number
		const RString& operator+=(short rhs)
			{
			Grow(m_lStrLen + MaxShortLen + 1); // size is always > 0, so this will always return with a valid buffer
			m_lStrLen += sprintf(m_pBuf + m_lStrLen, "%hd", (short)rhs);
			return *this;
			}

		// Append string representation of specified number
		const RString& operator+=(unsigned short rhs)
			{
			Grow(m_lStrLen + MaxUShortLen + 1); // size is always > 0, so this will always return with a valid buffer
			m_lStrLen += sprintf(m_pBuf + m_lStrLen, "%hu", (unsigned short)rhs);
			return *this;
			}

		// Append string representation of specified number
		const RString& operator+=(long rhs)
			{
			Grow(m_lStrLen + MaxLongLen + 1); // size is always > 0, so this will always return with a valid buffer
			m_lStrLen += sprintf(m_pBuf + m_lStrLen, "%ld", (long)rhs);
			return *this;
			}

		// Append string representation of specified number
		const RString& operator+=(unsigned long rhs)
			{
			Grow(m_lStrLen + MaxULongLen + 1); // size is always > 0, so this will always return with a valid buffer
			m_lStrLen += sprintf(m_pBuf + m_lStrLen, "%lu", (unsigned long)rhs);
			return *this;
			}

	//---------------------------------------------------------------------------
	// Operator +
	// This concatenates two RStrings, or an RString and another supported type,
	// and returns the result in a new RString.
	//---------------------------------------------------------------------------
	public:
		RString operator+(const RString& rhs) const
			{
			RString str = *this;
			str += rhs;
			return str;
			}

		RString operator+(const char* rhs) const
			{
			RString str = *this;
			str += rhs;
			return str;
			}

		RString operator+(char rhs) const
			{
			RString str = *this;
			str += rhs;
			return str;
			}

		RString operator+(short rhs) const
			{
			RString str = *this;
			str += rhs;
			return str;
			}

		RString operator+(unsigned short rhs) const
			{
			RString str = *this;
			str += rhs;
			return str;
			}

		RString operator+(long rhs) const
			{
			RString str = *this;
			str += rhs;
			return str;
			}

		RString operator+(unsigned long rhs) const
			{
			RString str = *this;
			str += rhs;
			return str;
			}

	//---------------------------------------------------------------------------
	// Comparison functions are implimented as friend functions so they'll work
	// regardless of which order the operands are arranged in.
	//---------------------------------------------------------------------------
	public:
		// RString -vs- RString
		friend int operator==(const RString& lhs, const RString& rhs);
		friend int operator!=(const RString& lhs, const RString& rhs);
//		friend int operator<(const RString& lhs, const RString& rhs);
		friend int operator>(const RString& lhs, const RString& rhs);
		friend int operator<=(const RString& lhs, const RString& rhs);
		friend int operator>=(const RString& lhs, const RString& rhs);

		// RString -vs- char*
		friend int operator==(const RString& lhs, const char* rhs);
		friend int operator==(const char* rhs, const RString& lhs);
	};


////////////////////////////////////////////////////////////////////////////////
// RString -vs- RString
////////////////////////////////////////////////////////////////////////////////
inline int operator==(const RString& lhs, const RString& rhs)
	{ if (strcmp(lhs, rhs) == 0) return 1; return 0; }
inline int operator!=(const RString& lhs, const RString& rhs)
	{ if (strcmp(lhs, rhs) != 0) return 1; return 0; }
inline int operator<=(const RString& lhs, const RString& rhs)
	{ if (strcmp(lhs, rhs) <= 0) return 1; return 0; }
inline int operator>=(const RString& lhs, const RString& rhs)
	{ if (strcmp(lhs, rhs) >= 0) return 1; return 0; }
inline bool operator<(const RString& lhs, const RString& rhs)
	{ if (strcmp(lhs, rhs) < 0) return true; return false; }
inline int operator>(const RString& lhs, const RString& rhs)
	{ if (strcmp(lhs, rhs) > 0) return 1; return 0; }

////////////////////////////////////////////////////////////////////////////////
// RString -vs- const char*
////////////////////////////////////////////////////////////////////////////////
inline int operator==(const RString& lhs, const char* rhs)
	{ if (strcmp(lhs, rhs) == 0) return 1; return 0; }
inline int operator!=(const RString& lhs, const char* rhs)
	{ if (strcmp(lhs, rhs) != 0) return 1; return 0; }
inline int operator<=(const RString& lhs, const char* rhs)
	{ if (strcmp(lhs, rhs) <= 0) return 1; return 0; }
inline int operator>=(const RString& lhs, const char* rhs)
	{ if (strcmp(lhs, rhs) >= 0) return 1; return 0; }
//inline int operator<(const RString& lhs, const char* rhs)
//	{ if (strcmp(lhs, rhs) < 0) return 1; return 0; }
inline int operator>(const RString& lhs, const char* rhs)
	{ if (strcmp(lhs, rhs) > 0) return 1; return 0; }

inline int operator==(const char* rhs, const RString& lhs)
	{ if (strcmp(lhs, rhs) == 0) return 1; return 0; }
inline int operator!=(const char* rhs, const RString& lhs)
	{ if (strcmp(lhs, rhs) != 0) return 1; return 0; }
inline int operator<=(const char* rhs, const RString& lhs)
	{ if (strcmp(lhs, rhs) <= 0) return 1; return 0; }
inline int operator>=(const char* rhs, const RString& lhs)
	{ if (strcmp(lhs, rhs) >= 0) return 1; return 0; }
//inline int operator<(const char* rhs, const RString& lhs)
//	{ if (strcmp(lhs, rhs) < 0) return 1; return 0; }
inline int operator>(const char* rhs, const RString& lhs)
	{ if (strcmp(lhs, rhs) > 0) return 1; return 0; }


#endif // RSTRING_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
