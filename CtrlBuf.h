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
// CtrlBuf.h
// Project: Nostril (aka Postal)
//
//	History:
//		05/26/97 MJR	Started.
//
//		06/15/97 MJR	Added Reset().
//
////////////////////////////////////////////////////////////////////////////////
#ifndef CTRLBUF_H
#define CTRLBUF_H

#include "RSPiX.h"


////////////////////////////////////////////////////////////////////////////////
//
// CCtrlBuf impliments a specialized buffer designed for random access.
// Also, when getting a series of values from the buffer, they must ALWAYS
// be in a single, contiguous chunk of memory, which is not the case with a
// traditional circular buffer.
//
// This is pretty crude right now, as it may result in a memmove() each time
// a value is discarded from the buffer.  One optimization might be to move
// the front of the buffer forward until the back hits the end of the
// available space, and only then do a memmove() of the remaining data back
// to the beginning of the buffer.  Alternately, maybe a traditional queue
// should be used as the core, with an intermediate buffer to take care of the
// special case where a chunk of data wraps around the end of the buffer.
//
////////////////////////////////////////////////////////////////////////////////
class CCtrlBuf
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:
		enum
			{
			MaxBufEntries = 256,
			InvalidEntry = 0xffffffff
			};

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:
		long m_lNumCtrls;
		long m_lOldestSeq;

		long m_alBuf[MaxBufEntries];

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CCtrlBuf()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CCtrlBuf()
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Reset
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void)
			{
			m_lNumCtrls = 0;
			m_lOldestSeq = 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Add new ctrl to buffer
		////////////////////////////////////////////////////////////////////////////////
		short Add(
			long lSeq,
			long lNum,
			long* plCtrls,
			long* plNumAdded)
			{
			short sResult = 0;

			// This can and should be optimized!
			*plNumAdded = 0;
			for (long l = 0; l < lNum; l++)
				{
				sResult = Add(lSeq++, *plCtrls++);
				if (sResult == 0)
					(*plNumAdded)++;
				else
					break;
				}

			return sResult;
			}

		short Add(
			long lSeq,
			long lCtrl)
			{
			short sResult = 0;

			// This needs to deal with the fact that the ctrls don't always come
			// sequentially, and there will often be gaps between ctrl values that
			// get filled in later on.
			if (m_lNumCtrls == 0)
				{
				// If array is empty, set oldest seq to specified seq, add ctrl to
				// array, and adjust the number of ctrls.
				m_lOldestSeq = lSeq;
				m_alBuf[0] = lCtrl;
				m_lNumCtrls++;
				}
			else
				{
				// Calculate index in array based on sequence number
				long lIndex = lSeq - m_lOldestSeq;

				// Make sure it isn't older than our oldest value.  This can happen if
				// we receive an older data packet.  If it does happen, we quietly
				// ignore the value since it is apparently no longer needed.
				if (lIndex >= 0)
					{
					// Make sure it will fit in the buffer
					if (lIndex < MaxBufEntries)
						{
						// Add new ctrl to array
						m_alBuf[lIndex] = lCtrl;

						// Check if new ctrl went beyond the "current" number of entries
						if (lIndex >= m_lNumCtrls)
							{
							// Invalidate any unused ctrls (there may not be any)
							for (long l = m_lNumCtrls; l < lIndex; l++)
								m_alBuf[l] = InvalidEntry;

							// Set new number of entries (last index + 1)
							m_lNumCtrls = lIndex + 1;
							}
						}
					else
						{
						sResult = -1;
						TRACE("No room in buf!\n");
						}
					}
				}
			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get specified entry.  If the returned value is 'InvalidEntry', then that
		// entry was not available.
		////////////////////////////////////////////////////////////////////////////////
		long GetAt(
			long lSeq)
			{
			if (m_lNumCtrls > 0)
				{
				long lIndex = lSeq - m_lOldestSeq;
				if ((lIndex >= 0) && (lIndex < m_lNumCtrls))
					return m_alBuf[lIndex];
				}
			return InvalidEntry;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get pointer to specified entry.  If the returned value is 0 (NULL), then
		// that entry was not available.
		////////////////////////////////////////////////////////////////////////////////
		long* GetPtrTo(
			long lSeq)
			{
			if (m_lNumCtrls > 0)
				{
				long lIndex = lSeq - m_lOldestSeq;
				if ((lIndex >= 0) && (lIndex < m_lNumCtrls))
					return &(m_alBuf[lIndex]);
				}
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Discard all ctrls up to the specified sequence number
		////////////////////////////////////////////////////////////////////////////////
		void DiscardThrough(
			long lSeq)
			{
			if (m_lNumCtrls > 0)
				{
				if (lSeq >= m_lOldestSeq)
					{
					// Calculate index of first ctrl that will be KEPT
					long lKeepIndex = (lSeq - m_lOldestSeq) + 1;

					// If index is less than number of ctrls, then there's something to be moved.
					// Otherwise, the entire array has been discarded.
					if (lKeepIndex < m_lNumCtrls)
						{
						// Move remaining ctrls down to start of array
						memmove(&(m_alBuf[0]), &(m_alBuf[lKeepIndex]), (m_lNumCtrls - lKeepIndex) * sizeof(m_alBuf[0]));
						m_lNumCtrls -= lKeepIndex;
						m_lOldestSeq = lSeq + 1;
						}
					else
						{
						// All entries were discarded
						m_lNumCtrls = 0;
						}
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine whether buffer is empty
		////////////////////////////////////////////////////////////////////////////////
		bool IsEmpty(void)
			{
			return m_lNumCtrls == 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine whether buffer is full
		////////////////////////////////////////////////////////////////////////////////
		bool IsFull(void)
			{
			return m_lNumCtrls == MaxBufEntries;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get oldest sequence (not valid if buffer is empty!)
		////////////////////////////////////////////////////////////////////////////////
		long GetOldestSeq(void)
			{
			return m_lOldestSeq;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get newest sequence (not valid if buffer is empty!)
		////////////////////////////////////////////////////////////////////////////////
		long GetNewestSeq(void)
			{
			return m_lOldestSeq + (m_lNumCtrls - 1);
			}
	};


#endif //CTRLBUF_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
