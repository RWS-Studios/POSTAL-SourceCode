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
// BufQ.H
// 
// History:
//		05/24/97 JMI	Started.
//
//		05/24/97	JMI	Added UnGet().
//
//		08/14/97 MJR	Moved everything into this header so that it would
//							inline, which is highly likely considering the small
//							size of these functions.  Also reworked the endian
//							stuff so that the basic nature of the bufQ is to store
//							data in standard "network order", which is big endian.
//
//		08/15/97 MJR	Changed GetFree() and GetUsed() to return non-linear
//							values instead of linear values.
//
//					MJR	Lots more cleaning up and fixed several serious bugs
//							that resulted from incorrect calculations of "linear"
//							bytes.
//
//////////////////////////////////////////////////////////////////////////////
//
// This class is designed as a simple means of accessing a circular buffer
// in via two types of interfaces:  { Get (deQ), Put (enQ) } and/or { LockBuf,
// ReleaseBuf } styles.
// Potentially bad schtuff:
//		- Safety on bad behavior for LockBuf/ReleaseBuf pairs is low.
//		- Since different types of data can be written w/o this class 
//			storing what type it was, reads must be done with the same care
//			used when accessing files (i.e., either know the format of the data
//			explicitly or 'learn' it by data context).
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BUFQ_H
#define BUFQ_H

#include "System.h"

//////////////////////////////////////////////////////////////////////////////
//
// This impliments a fast buffer queue.
//
// The queue is basically a circular buffer with a get position and a put
// position.
//
// The get and put positions both start out at the same position, which
// happens to be 0.  Whenever the get and put positions are the same,
// the queue is empty.
//
// Each time we put a byte to the queue, we put it at the current put
// position and then increment the put position.
//
// Each time we get a byte from the queue, we get it from the current get
// position and then increment the get position.
//
// When either position reaches the end of the queue, it wraps back around to
// the beginning.
//
// When the put position is moved forward so many times that it "wraps
// around" and ends up one position before the get position, then the queue
// is considered full.
//
// Note that because of the way we detect whether the queue is full, we
// need the queue's memory to be one byte larger than the desired "volume"
// of the queue.  In other words, if you want the queue to hold 100 bytes,
// the memory buffer must be 101 bytes.  Let's say both the get and put
// positions start at 0 and we put 100 bytes into the queue.  The put
// position will now be at 100 (0 through 99 have been written to).  The
// put position is now 1 position before the get position, and the queue
// is full.  If we incremented the put position again, it would wrap around
// and become equal to the get position.  We can't let that happen, because
// then we would consider the queue to be empty!  Hence, we need that extra
// byte to differentiate between empty and full.
//
//////////////////////////////////////////////////////////////////////////////
class CBufQ
	{
	//----------------------------------------------------------------------------
	// Macros
	//----------------------------------------------------------------------------

	// Calculate the next position in the queue, relative to the specified position 'i'
	#define BUFQ_NEXT(i) 	(((i)+(short)1) >= QueueSize ? (short)0 : ((i)+(short)1))
	
	// Calculate the previous position in the queue, relative to the specified position 'i'
	#define BUFQ_PREV(i)		(((i)-(short)1) >= 0 ? ((i)-(short)1) : (QueueSize-(short)1))

	//----------------------------------------------------------------------------
	// Types, enums, etc.
	//----------------------------------------------------------------------------
	public:
		enum
			{
			// This is the volume of the queue.  In other words, how many bytes
			// it can actually hold before it is considered "full".
			QueueVolume	= 4096,
			
			// This is the actual size (in bytes) of the queue's memory.  It is
			// one larger than the volume so that we can tell the difference
			// between it being empty and full.  See above for details.
			QueueSize
			};

	//----------------------------------------------------------------------------
	// Variables
	//----------------------------------------------------------------------------
	public:
		U8			m_au8Buf[QueueSize];		// Buffer.
		short		m_sPutPos;					// Current put position in buffer.
		short		m_sGetPos;					// Current get position in buffer.

	//----------------------------------------------------------------------------
	// Functions
	//----------------------------------------------------------------------------
	public:

		///////////////////////////////////////////////////////////////////////////////
		// Default (and only) constructor
		///////////////////////////////////////////////////////////////////////////////
		CBufQ()
			{
			Reset();
			}

		///////////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////////
		~CBufQ()
			{
			}

		///////////////////////////////////////////////////////////////////////////////
		// Reset the queue
		///////////////////////////////////////////////////////////////////////////////
		void Reset(void)				// Returns nothing.
			{
			m_sPutPos	= 0;
			m_sGetPos	= 0;
			}

		///////////////////////////////////////////////////////////////////////////////
		// Simple queries
		///////////////////////////////////////////////////////////////////////////////

		// Determine whether queue is full
		bool IsFull(void)
			{
			return BUFQ_NEXT(m_sPutPos) == m_sGetPos;
			}

		// Determine whether queue is empty
		bool IsEmpty(void)
			{
			return m_sGetPos == m_sPutPos;
			}

		// Determine whether we can Put() another byte
		bool CanPutByte(void)
			{
			return BUFQ_NEXT(m_sPutPos) != m_sGetPos;
			}

		// Determine whether we can Get() another byte
		bool CanGetByte(void)
			{
			return m_sGetPos != m_sPutPos;
			}

		// Check how many bytes can be gotten via Get()
		long CheckGetable(void)
			{
			// Calculate total amount of used space in queue
			return (m_sGetPos <= m_sPutPos) ? m_sPutPos - m_sGetPos : QueueSize - m_sGetPos + m_sPutPos;
			}

		// Check how many bytes can be put via Put()
		long CheckPutable(void)
			{
			// Calculate total amount of free space in queue
			return (m_sPutPos < m_sGetPos) ? m_sGetPos - (m_sPutPos + 1) : QueueSize - (m_sPutPos + 1) + m_sGetPos;
			}

		// Check how many bytes can be gotten via LockGetPtr()
		long CheckLockGetPtr(void)
			{
			// We need to figure out how many bytes can be gotten from the queue
			// without wrapping around or hitting the put position.
			return (m_sGetPos <= m_sPutPos) ? m_sPutPos - m_sGetPos : QueueSize - m_sGetPos;
			}

		// Check how many bytes can be put via LockPutPtr()
		long CheckLockPutPtr(void)
			{
			// We need to figure out how many bytes can be put to the queue without
			// wrapping around or hitting the get position.  A special case exists
			// when the get position is at 0, because then we can't fill out to the
			// end of the queue, but must instead stop one byte before the end.
			long lPutable;
			if (m_sPutPos < m_sGetPos)
				lPutable = m_sGetPos - (m_sPutPos + 1);
			else
				{
				if (m_sGetPos)
					lPutable = QueueSize - m_sPutPos;
				else
					lPutable = QueueSize - (m_sPutPos + 1);
				}
			return lPutable;
			}

		///////////////////////////////////////////////////////////////////////////////
		// Various flavors of Put()
		///////////////////////////////////////////////////////////////////////////////
		long Put(				// Returns 1 if it fit, 0 if not enough room in queue
			U8 u8Val)			// In:  Data to enqueue in buffer.
			{
			long lResult = 0;
			if (CanPutByte())
				{
				*(m_au8Buf + m_sPutPos) = u8Val;
				m_sPutPos = BUFQ_NEXT(m_sPutPos);
				lResult++;
				}
			return lResult;
			}

		long Put(				// Returns number of items that were put into queue
			U8* pu8Buf,			// In:  Data to enqueue in buffer.
			long lNum = 1)		// In:  Number of bytes to put.
			{
			long	lNumPut	= -1;
			while (++lNumPut < lNum)
				{
				if (!Put(*pu8Buf++))
					break;
				}
			return lNumPut;
			}

		long Put(				// Returns 1 if it fit, 0 if not enough room in queue
			S8 s8Val)			// In:  Data to enqueue in buffer.
			{
			return Put((U8)s8Val);
			}

		long Put(				// Returns number of items that were put into queue
			S8* ps8Buf,			// In:  Data to enqueue in buffer.
			long lNum = 1)		// In:  Number of bytes to put.
			{
			return Put((U8*)ps8Buf, lNum);
			}

		long Put(				// Returns number of items that were put into queue
			void* pvBuf,		// In:  Data to enqueue in buffer.
			long lNum)			// In:  Number of bytes to put.
			{
			return Put((U8*)pvBuf, lNum);
			}

		long Put(				// Returns number of items that were put into queue
			U16* pu16Buf,		// In:  Data to enqueue in buffer.
			long lNum = 1)		// In:  Number of U16s to put.
			{
			long	lNumPut	= -1;
			U8*	pu8Buf	= (U8*)pu16Buf;

			#ifdef SYS_ENDIAN_BIG
				while (++lNumPut < lNum)
					{
					Put(*pu8Buf++);
					if (!Put(*pu8Buf++))
						break;
					}
			#else
				while (++lNumPut < lNum)
					{
					Put(*(pu8Buf + 1));
					if (!Put(*(pu8Buf + 0)))
						break;
					pu8Buf	+= 2;
					}
			#endif
			
			return lNumPut;
			}
									
		long Put(				// Returns number of items that were put into queue
			S16* ps16Buf,		// In:  Data to enqueue in buffer.
			long lNum = 1)		// In:  Number of S16s to put.
			{
			return Put((U16*)ps16Buf, lNum);
			}

		long Put(				// Returns 1 if it fit, 0 if not enough room in queue
			U16 u16Val)			// In:  Data to enqueue in buffer.
			{
			return Put(&u16Val);
			}

		long Put(				// Returns 1 if it fit, 0 if not enough room in queue
			S16 s16Val)			// In:  Data to enqueue in buffer.
			{
			return Put(&s16Val);
			}

		long Put(				// Returns number of items that were put into queue
			U32* pu32Buf,		// In:  Data to enqueue in buffer.
			long lNum = 1)		// In:  Number of U32s to put.
			{
			long	lNumPut	= -1;
			U8*	pu8Buf	= (U8*)pu32Buf;

			#ifdef SYS_ENDIAN_BIG
				while (++lNumPut < lNum)
					{
					Put(*pu8Buf++);
					Put(*pu8Buf++);
					Put(*pu8Buf++);
					if (!Put(*pu8Buf++))
						break;
					}
			#else
				while (++lNumPut < lNum)
					{
					Put(*(pu8Buf + 3));
					Put(*(pu8Buf + 2));
					Put(*(pu8Buf + 1));
					if (!Put(*(pu8Buf + 0)))
						break;
					pu8Buf	+= 4;
					}
			#endif

			return lNumPut;
			}
									
		long Put(				// Returns number of items that were put into queue
			S32* ps32Buf,		// In:  Data to enqueue in buffer.
			long lNum = 1)		// In:  Number of S32s to put.
			{
			return Put((U32*)ps32Buf, lNum);
			}

		long Put(				// Returns 1 if it fit, 0 if not enough room in queue
			U32 u32Val)			// In:  Data to enqueue in buffer.
			{
			return Put(&u32Val);
			}

		long Put(				// Returns 1 if it fit, 0 if not enough room in queue
			S32 s32Val)			// In:  Data to enqueue in buffer.
			{
			return Put(&s32Val);
			}

		///////////////////////////////////////////////////////////////////////////////
		// Various flavors of Get()
		///////////////////////////////////////////////////////////////////////////////
		long Get(				// Returns 1 if item was dequeued, 0 otherwise
			U8* pu8Val)			// Out: Where to dequeue from buffer.
			{
			long lNumGot = 0;
			if (CanGetByte())
				{
				*pu8Val = *(m_au8Buf + m_sGetPos);
				m_sGetPos = BUFQ_NEXT(m_sGetPos);
				lNumGot++;
				}
			return lNumGot;
			}
									
		long Get(				// Returns number of items dequeued.
			U8* pu8Buf,			// Out: Where to dequeue from buffer.
			long lNum)			// In:  Number of bytes to get.
			{
			long	lNumGot	= -1;
			while (++lNumGot < lNum)
				{
				if (!Get(pu8Buf++))
					break;
				}
			return lNumGot;
			}

		long Get(				// Returns number of items dequeued.
			S8* ps8Buf,			// Out: Where to dequeue from buffer.
			long lNum = 1)		// In:  Number of bytes to get.
			{
			return Get((U8*)ps8Buf, lNum);
			}

		long Get(				// Returns number of items dequeued
			void* pvBuf,		// Out: Where to dequeue from buffer.
			long lNum)			// In:  Number of bytes to get.
			{
			return Get((U8*)pvBuf, lNum);
			}

		long Get(				// Returns number of items dequeued.
			U16* pu16Buf,		// Out: Where to dequeue from buffer.
			long lNum = 1)		// In:  Number of U16s to get.
			{
			long	lNumGot	= -1;
			U8*	pu8Buf	= (U8*)pu16Buf;

			#ifdef SYS_ENDIAN_BIG
				while (++lNumGot < lNum)
					{
					Get(pu8Buf++);
					if (!Get(pu8Buf++))
						break;
					}
			#else
				while (++lNumGot < lNum)
					{
					Get(pu8Buf + 1);
					if (!Get(pu8Buf + 0))
						break;
					pu8Buf	+= 2;
					}
			#endif

			return lNumGot;
			}

		long Get(				// Returns number of items dequeued.
			S16* ps16Buf,		// Out: Where to dequeue from buffer.
			long lNum = 1)		// In:  Number of S16s to get.
			{
			return Get((U16*)ps16Buf, lNum);
			}

		long Get(				// Returns number of items dequeued.
			U32* pu32Buf,		// Out: Where to dequeue from buffer.
			long lNum = 1)		// In:  Number of U32s to get.
			{
			long	lNumGot	= -1;
			U8*	pu8Buf	= (U8*)pu32Buf;

			#ifdef SYS_ENDIAN_BIG
				while (++lNumGot < lNum)
					{
					Get(pu8Buf++);
					Get(pu8Buf++);
					Get(pu8Buf++);
					if (!Get(pu8Buf++))
						break;
					}
			#else
				while (++lNumGot < lNum)
					{
					Get(pu8Buf + 3);
					Get(pu8Buf + 2);
					Get(pu8Buf + 1);
					if (!Get(pu8Buf + 0))
						break;
					pu8Buf	+= 4;
					}
			#endif

			return lNumGot;
			}

		long Get(				// Returns number of items dequeued.
			S32* ps32Buf,		// Out: Where to dequeue from buffer.
			long lNum = 1)		// In:  Number of S32s to get.
			{
			return Get((U32*)ps32Buf, lNum);
			}

		///////////////////////////////////////////////////////////////////////////////
		// Un-put a byte.  Can be called repeatedly until all data has been "removed".
		// Use caution when mixing with other functions, especially UnGet().
		///////////////////////////////////////////////////////////////////////////////
		short UnPut(void)								// Returns 1 if able to unput, 0 if nothing to unput
			{
			short sResult = 0;
			// Being able to get a byte also happens to indicate that we can unput a byte!
			// In other words, if we can move the get pointer forward 1 byte, it means we
			// can instead move the put pointer back 1 byte.  Get it?
			if (CanGetByte())
				{
				// Move the put position back by 1 byte
				m_sPutPos = BUFQ_PREV(m_sPutPos);
				sResult++;
				}
			return sResult;
			}

		///////////////////////////////////////////////////////////////////////////////
		// Un-Get a byte.  Can be called repeatedly until all data has been "restored".
		// Use caution when mixing with other functions, especially UnPut().
		///////////////////////////////////////////////////////////////////////////////
		short UnGet(void)								// Returns 1 if able to unget, 0 if nothing to unget
			{
			short sResult = 0;
			// Being able to put a byte also happens to indicate that we can unput a byte!
			// In other words, if we can move the put pointer forward 1 byte, it means we
			// can instead move the get pointer back 1 byte.  Get it?
			if (CanPutByte())
				{
				// Move the get position back by 1 byte
				m_sGetPos = BUFQ_PREV(m_sGetPos);
				sResult++;
				}
			return sResult;
			}

		///////////////////////////////////////////////////////////////////////////////
		// Lock down the put ptr so you can write directly into the buffer.
		// Don't forget to release this ptr with ReleasePutPtr()!!!!
		///////////////////////////////////////////////////////////////////////////////
		void LockPutPtr(
			U8** ppu8Put,				// Out: Pointer to which up to *plAmountAvail bytes can be put
			long* plAvail)				// Out: Number of bytes that can be put to above pointer
			{
			*plAvail = CheckLockPutPtr();
			*ppu8Put = m_au8Buf + m_sPutPos;
			}

		///////////////////////////////////////////////////////////////////////////////
		// Release a put ptr previously locked with LockPutPtr().
		// This function is indiscriminant and will screw you if you lie!
		///////////////////////////////////////////////////////////////////////////////
		void ReleasePutPtr(
			long	lBytes)				// In:  Number of bytes written to locked pointer
			{
			ASSERT(lBytes <= CheckLockPutPtr());
			m_sPutPos += lBytes;
			if (m_sPutPos == QueueSize)
				m_sPutPos = 0;
			}

		///////////////////////////////////////////////////////////////////////////////
		// Lock down the get ptr so you can read directly from the buffer.
		// Don't forget to release this ptr with ReleaseGetPtr()!!!!
		///////////////////////////////////////////////////////////////////////////////
		void LockGetPtr(
			U8** ppu8Get,				// Out: Pointer from which up to *plAmountAvail bytes can be gotten
			long* plAvail)				// Out: Number of bytes that can be gotten from above pointer
			{
			*plAvail = CheckLockGetPtr();
			*ppu8Get = m_au8Buf + m_sGetPos;
			}

		///////////////////////////////////////////////////////////////////////////////
		// Release a get ptr previously locked with LockGetPtr().
		// This function is indiscriminant and will screw you if you lie!
		///////////////////////////////////////////////////////////////////////////////
		void ReleaseGetPtr(
			long	lBytes)				// In:  Number of bytes that were gotten from locked pointer
			{
			ASSERT(lBytes <= CheckLockGetPtr());
			m_sGetPos += lBytes;
			if (m_sGetPos == QueueSize)
				m_sGetPos = 0;
			}
	};

#endif	// BUFQ_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
