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
// string.cpp
// Project: Nostril (aka Postal)
//
// History:
//		01/17/97 MJR	Started.
//		01/18/97 MJR	First (tentative) release.
//		01/20/97 MJR	Revised lots of stuff, ready for testing/prerelease.
//		01/21/97 MJR	Did a bunch of testing, fixed one minor typo-bug.
//		01/28/97 MJR	Fixed ASSERT(1) to be ASSERT(0), as it should have been.
//							Added char* operator.
//							Added Update() function.
//		02/10/97 MJR	Minor tweak to Save() -- got rid of (strange) unecessary
//							cast.  Also got rid of one constructor varation which had
//							allowed you to create an RString based on the value of a
//							char.  The problem was that this caused an "abiguous"
//							overload when you were trying to create an RString of
//							a specified length, as in RString str(5).  Hopefully the
//							removed constructor will not be missed.
//
//		06/28/97 MJR	Fixed typo in mac-specific code that had never been
//							compiled before (naturally).
//
//		07/12/97 MJR	RString::Load() didn't initialize sResult, so it would
//							return random error codes instead of 0 for success.
//
////////////////////////////////////////////////////////////////////////////////
//
// The primary purpose of RString is to make it easier to work with text strings
// by hiding the memory and storage aspects from the user.  It lets the user
// focus on the real task without getting caught up in the nasty details.
//
//
// Most RString functions and operators are overloaded to accept any of the
// following types.  These are often referred to as the "supported types".
//
//		* Other RString's
//
//		* C-style strings (char*)
//
//		* Characters (char), which are treated as ASCII codes
//
//		* Integers (short, unsigned short, long, and unsigned long) converted
//		  into string representations of their numerical values.  The exception
//		  is that there are no constructors to take these types, only because
//		  it caused a conflict with other, more usefull constructors.
//
//
// This is an overview of the available functions.  Check the comments on
// the individual functions for details.
//
//		RString()		Various constructors that allow the creation of a string,
//							optionally based on any supported type, and optionally
//							with a specific buffer size
//
//		~RString()		Destructor
//
//		GetSize()		Get size of buffer (always larger than string length)
//
//		Grow()			Grow buffer to at least the specified size
//
//		Shrink()			Shrink buffer to at most the specified size
//
//		Compact()		Compact buffer to minimum size required by current string
//
//		Clear()			Clear string (length will be 0)
//
//		GetLen()			Get string length
//
//		GetAt()			Get character at specified position in string
//
//		operator[]		Get character at specified position in string
//
//		SetAt()			Set character at specified position in string
//
//		operator char*	Casts RString to C-style string
//
//		Update()			Updates string if it was modified via the char* pointer
//
//		Format()			Create string using sprintf-like method
//
//		Left()			Returns new RString based on a portion of this string
//
//		Right()			Returns new RString based on a portion of this string
//
//		Mid()				Returns new RString based on a portion of this string
//
//		Range()			Returns new RString based on a portion of this string
//
//		Insert()			Insert any supported type into string
//
//		Delete()			Delete portion of string
//
//		ToUpper()		Convert string to upper case
//
//		ToLower()		Convert string to lower case
//
//		Load()			Load previously saved string
//
//		Save()			Save string
//
//		operator=		Assigns any supported type into string
//
//		operator+=		Appends supported types onto current string
//
//		operator+		Concatinates string and any supported type
//
//		operator==		Full gamut of comparison operators
//		operator!=
//		operator<=
//		operator>=
//		operator<
//		operator>
//
//
// Functions to be added some time soon...
//
//		I'm thinking Insert() could be easily extended to make it much more
//		usefull -- see the Insert() comment block (in the header file!) for
//		details.
//
//		Along the same lines as Insert(), I think a Copy() would be usefull.
//		It would be just like Insert() except it would overwrite the data in
//		the destination string.
//
//		Together, just about everything else could be implimented based on
//		Insert() and Copy()!  I'm not sure it's worth switching over to that,
//		though.  Too bad I didn't create them first.
//
//
// More Information
//
// An RString object will automatically increase the size of its memory buffer
// as necessary to accomodate the growth of the text string it holds.  It will
// never reduce the buffer's size automatically, although the user can trigger
// such a reduction at any time by calling Compact().
//
// The user can also explicitly set the size of the buffer, although it will
// still be grown automatically if necessary.  The idea is to allow the user to
// make the buffer large enough so that additional growth isn't necessary,
// thereby improving performance and reducing memory fragmentation.
//
// When working with RStrings, knowing the length of a string becomes much less
// important than it is with C-style strings.  In light of this, all RString
// functions that take a length as a parameter will automatically clip that
// length to the actual size of the string.  For instance, if the first 8
// characters of a string are requested but the string is only 5 characters
// long, then the requested length is clipped, and only 5 characters are
// returned.
//
// In constract, NO clipping is performed at the beginning of a string.  The
// first character of a string is always at position 0.  If a negative position
// is passed to any RString function, it will do nothing or return an empty
// RString, whichever is appropriate for that function.  Negative positions
// are basically considered meaningless, but are handled without any complaints
// (no TRACE's or ASSERT's), the idea being to leave it up to the user to
// decide whether to validate positions beforehand or to check results
// afterwards.
//
// Put another way, functions will generally clip lengths to fit 
// When an RString function takes a length as a parameter, that length will
// be truncated if necessary to avoid going past the end of the string.  When
// a function takes a position as a parameter, the function won't do anything
// if that position is negative.
//
// As a general rule, clipping is performed on the right of strings but not
// on the left end.  The reason for these two different approaches is actually
// pretty straightforward.
//
////////////////////////////////////////////////////////////////////////////////
#define RSTRING_CPP

#include <ctype.h>
#include "rstring.h"


// Static pointer that is initialized to point at an empty string.  Used
// in cases where string is being cast as char* but doesn't have a buffer
// of it's own.
char* RString::ms_pszEmpty = "";


////////////////////////////////////////////////////////////////////////////////
// Grow the buffer to the specified size.  If the buffer is already greater than
// or equal to the specified size, then this will have no effect.  Otherwise,
// the buffer is grown to the specified size.  The string is unaffected.
////////////////////////////////////////////////////////////////////////////////
void RString::Grow(long lMinimumSize)
	{
	// New size must be greater than current size (which might be 0)
	// (This comparison also filters out negative sizes, which are stupid)
	if (lMinimumSize > m_lBufSize)
		{
		if (m_lBufSize > 0)
			{
			// Change buffer size
			char* pOld = m_pBuf;
			m_pBuf = (char*)malloc(lMinimumSize);
			ASSERT(m_pBuf != 0); // should be caught by new_handler, but just in case...
			memcpy(m_pBuf, pOld, m_lBufSize);
			free(pOld);
			}
		else
			{
			// Create new buffer
			m_pBuf = (char*)malloc(lMinimumSize);
			ASSERT(m_pBuf != 0); // should be caught by new_handler, but just in case...
			*m_pBuf = 0;	// write terminating null (string length must be 0 at this point)
			}
		m_lBufSize = lMinimumSize;
		}
	}


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
void RString::Shrink(long lMaximumSize)
	{
	// Guard against stupid values
	if (lMaximumSize >= 0)
		{
		// New size must be less than current size
		// (This comparison also ensures that m_lBufSize > 0, meaning there is a buffer)
		if (lMaximumSize < m_lBufSize)
			{
			if (lMaximumSize == 0)
				{
				FreeBuf();
				}
			else
				{
				// Change buffer size
				char* pOld = m_pBuf;
				m_pBuf = (char*)malloc(lMaximumSize);
				ASSERT(m_pBuf != 0); // should be caught by new_handler, but just in case...
				memcpy(m_pBuf, pOld, m_lBufSize);
				free(pOld);
				m_lBufSize = lMaximumSize;
				// See if we truncated the string
				if (m_lBufSize <= m_lStrLen)
					{
					// Write new terminating null at end of buffer
					m_pBuf[m_lBufSize - 1] = 0;
					// Set new string length
					m_lStrLen = m_lBufSize - 1;
					}
				}
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Compact the buffer to the minimum size required to hold the current string.
// If the string is empty, the buffer is freed.
////////////////////////////////////////////////////////////////////////////////
void RString::Compact(void)
	{
	// If there's a string, shrink buffer to fit it, otherwise get rid of it
	if (m_lStrLen > 0)
		Shrink(m_lStrLen + 1); // shrink buffer (if necessary) to 1 larger than string
	else
		FreeBuf();
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
long RString::Format(long lMinimumSize, char* format, ...)
	{
	long lWritten = 0;
	if (lMinimumSize >= 0)
		{
		// Grow buffer
		Grow(lMinimumSize);
		// Since minimum size might be 0, we have to make sure buffer exists
		if (m_lBufSize > 0)
			{
			va_list varp;
			va_start(varp, format);
			#ifdef _MSC_VER
				lWritten = _vsnprintf(m_pBuf, m_lBufSize - 1, format, varp);
			#else
				lWritten = vsprintf(m_pBuf, format, varp);
				#ifdef _DEBUG
					if ((lWritten >= 0) && ((lWritten + 1) > m_lBufSize))
						{
						TRACE("RString::Format(): String buffer was overwritten!  String will be cleared!\n");
						Clear();
						ASSERT(0); // memory has been corrupted!
						}
				#endif
			#endif
			va_end(varp);

			if (lWritten >= 0)
				{
				m_lStrLen = lWritten;
				}
			else
				{
				TRACE("RString::Format(): Error returned by vsprintf()! String will be cleared!\n");
				Clear();
				}
			}
		}
	return lWritten;
	}


////////////////////////////////////////////////////////////////////////////////
// Create a new RString based on this string's first 'lLen' characters.  If
// lLen is negative, the returned string will be empty.  Otherwise, the returned
// string's length will be lLen or this string's length, whichever is less.
////////////////////////////////////////////////////////////////////////////////
RString RString::Left(long lLen) const
	{
	RString str;
	if ((lLen > 0) && (m_lStrLen > 0))
		{
		if (lLen > m_lStrLen)
			lLen = m_lStrLen;
		str.Grow(lLen + 1); // size is always > 0, so this will always return with a valid buffer
		memcpy(str.m_pBuf, m_pBuf, lLen);
		str.m_pBuf[lLen] = 0;
		str.m_lStrLen = lLen;
		}
	return str;
	}


////////////////////////////////////////////////////////////////////////////////
// Create a new RString based on this string's last 'lLen' characters.  If lLen
// is negative, the returned string will be empty.  Otherwise, the returned
// string's length will be lLen or this string's length, whichever is less.
////////////////////////////////////////////////////////////////////////////////
RString RString::Right(long lLen) const
	{
	RString str;
	if ((lLen > 0) && (m_lStrLen > 0))
		{
		if (lLen > m_lStrLen)
			lLen = m_lStrLen;
		str.Grow(lLen + 1); // size is always > 0, so this will always return with a valid buffer
		memcpy(str.m_pBuf, m_pBuf + (m_lStrLen - lLen), lLen);
		str.m_pBuf[lLen] = 0;
		str.m_lStrLen = lLen;
		}
	return str;
	}


////////////////////////////////////////////////////////////////////////////////
// Create a new RString based on a portion of this string, starting at lPos and
// proceeding for lLen characters.  If lPos is negative or beyond the end of
// this string, or if lLen is negative, the returned string will be empty.  The
// specified lLen will be clipped if necessary to avoid going past the end
// of this string.
////////////////////////////////////////////////////////////////////////////////
RString RString::Mid(long lPos, long lLen) const
	{
	RString str;
	if ((lPos >= 0) && (lPos < m_lStrLen) && (lLen > 0)) // implies m_lStrLen > 0
		{
		if ((lPos + lLen) > m_lStrLen)
			lLen = m_lStrLen - lPos;
		str.Grow(lLen + 1); // size is always > 0, so this will always return with a valid buffer
		memcpy(str.m_pBuf, m_pBuf + lPos, lLen);
		str.m_pBuf[lLen] = 0;
		str.m_lStrLen = lLen;
		}
	return str;
	}


////////////////////////////////////////////////////////////////////////////////
// Create a new RString based on the specified range of characters from this
// string.  If either position is negative the returned string will be empty.
// The positions are clipped if they are past the end of this string.  lPos1
// must be less than or equal to lPos2.
////////////////////////////////////////////////////////////////////////////////
RString RString::Range(long lPos1, long lPos2) const
	{
	RString str;
	if ((lPos1 >= 0) && (lPos1 <= lPos2) && (lPos1 < m_lStrLen)) // implies m_lStrLen > 0
		{
		if (lPos2 >= m_lStrLen)
			lPos2 = m_lStrLen - 1;
		long lLen = (lPos2 - lPos1) + 1;
		str.Grow(lLen + 1); // size is always > 0, so this will always return with a valid buffer
		memcpy(str.m_pBuf, m_pBuf + lPos1, lLen);
		str.m_pBuf[lLen] = 0;
		str.m_lStrLen = lLen;
		}
	return str;
	}


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
void RString::Insert(long lPos, const RString& str)
	{
	if ((lPos >= 0) && (lPos <= m_lStrLen) && (str.m_lStrLen > 0))
		{
		Grow(m_lStrLen + str.m_lStrLen + 1);
		// Shift existing characters to the right (including terminating null!)
		memmove(m_pBuf + lPos + str.m_lStrLen, m_pBuf + lPos, (m_lStrLen + 1) - lPos);
		// Copy new characters
		memcpy(m_pBuf + lPos, str.m_pBuf, str.m_lStrLen);
		m_lStrLen += str.m_lStrLen;
		}
	}

void RString::Insert(long lPos, const char* psz)
	{
	long lLen = strlen(psz);
	if ((lPos >= 0) && (lPos <= m_lStrLen) && (lLen > 0))
		{
		Grow(m_lStrLen + lLen + 1);
		// Shift existing characters to the right (including terminating null!)
		memmove(m_pBuf + lPos + lLen, m_pBuf + lPos, (m_lStrLen + 1) - lPos);
		// Copy new characters
		memcpy(m_pBuf + lPos, psz, lLen);
		m_lStrLen += lLen;
		}
	}

void RString::Insert(long lPos, char c)
	{
	if ((lPos >= 0) && (lPos <= m_lStrLen))
		{
		Grow(m_lStrLen + 1 + 1);
		// Shift existing characters to the right (including terminating null!)
		memmove(m_pBuf + lPos + 1, m_pBuf + lPos, (m_lStrLen + 1) - lPos);
		// Copy new character
		m_pBuf[lPos] = c;
		m_lStrLen++;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Delete the specified number of characters starting at the specified position.
// If the position or length are negative, the string is left unmodified.
////////////////////////////////////////////////////////////////////////////////
void RString::Delete(long lPos, long lLen)
	{
	if ((lPos >= 0) && (lPos < m_lStrLen) && (lLen > 0)) // implies m_lStrLen > 0
		{
		if ((lPos + lLen) > m_lStrLen)
			lLen = m_lStrLen - lPos;
		// Shift existing characters to the left (including terminating null!)
		memmove(m_pBuf + lPos, m_pBuf + lPos + lLen, (m_lStrLen + 1) - (lPos + lLen));
		m_lStrLen -= lLen;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Convert string to upper case
////////////////////////////////////////////////////////////////////////////////
void RString::ToUpper(void)
	{
	long lLen = m_lStrLen;
	char* p = m_pBuf;
	if (lLen > 0)
		{
		do	{
			*p = toupper(*p);
			p++;
			} while (--lLen);
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Convert string to lower case
////////////////////////////////////////////////////////////////////////////////
void RString::ToLower(void)
	{
	long lLen = m_lStrLen;
	char* p = m_pBuf;
	if (lLen > 0)
		{
		do {
			*p = tolower(*p);
			p++;
			} while (--lLen);
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Load a previously saved string from the specified RFile.  Calls Clear() if
// an error occurs while loading.  Returns 0 if successfull, non-zero otherwise.
////////////////////////////////////////////////////////////////////////////////
short RString::Load(RFile* pFile)
	{
	short sResult = 0;

	// Read length to separate var to avoid corrupting real one in case of read error
	long lLen;
	if (pFile->Read(&lLen) == 1)
		{
		// Check if there's more to read
		if (lLen > 0)
			{
			// Grow buffer
			Grow(lLen + 1);
			if (pFile->Read(m_pBuf, lLen) == lLen)
				{
				// Add null and set length
				m_pBuf[lLen] = 0;
				m_lStrLen = lLen;
				}
			else
				{
				Clear();
				TRACE("RString::Load(): Error reading string!\n");
				sResult = -1;
				}
			}
		else
			{
			Clear();
			}
		}
	else
		{
		Clear();
		TRACE("RString::Load(): Error reading string length!\n");
		sResult = -1;
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save this string to the specified RFile.  This RString is not modified by the
// save, even if an error occurs.  Returns 0 if successfull, non-zero otherwise.
////////////////////////////////////////////////////////////////////////////////
short RString::Save(RFile* pFile) const
	{
	// Save string length
	pFile->Write(m_lStrLen);

	// See if there's more to save
	if (m_lStrLen > 0)
		pFile->Write(m_pBuf, m_lStrLen);

	// Check for errors
	if (pFile->Error())
		{
		TRACE("RString::Save(): Error writing string!\n");
		return -1; // error
		}

	return 0; // success
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
