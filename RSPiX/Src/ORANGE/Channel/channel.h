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
// channel.h
// Project: RSPiX
//
//	History:
//		01/??/97	MJR	Created.
//
//		02/10/97	JMI	Changed Load() and Save() to use rspAnyLoad() and
//							rspAnySave().
//
//		02/17/97 MJR	Added looping features, but the currently selected looping
//							mode is not currently loaded or saved because we don't
//							want to invalidate existing files.
//
//		02/18/97 MJR	Fixed stupid typo (LoopAtStart was set to 0 instead of 1).
//
//		05/17/97	JMI	Made GetAtTime() in RChannelSpline return void* instead of
//							datat b/c the base class defined it that way.  This didn't
//							show up in VC 4.2 b/c it does not expand unused templates.
//
//		06/27/97	JMI	#if 0'ed out RChannelSet which does not specify the 
//							template arguments to its base class RChannel.
//
//		07/12/97 MJR	Added file ID and version to file format while maintaining
//							ability to load older files that don't have this info.
//							Added new variation called RChannelCompressible.
//							Now loads and saves looping variables.
//
//		07/13/97 MJR	Switched over to a whole new approach that shields the
//							user from having to what type of channel to use ahead of
//							time.  RChannel is now the only class used by the user,
//							and it can handle uncompressed and compressed data.
//
//		09/04/97	JMI	~RChannel() was not delete'ing m_pcore.  Fixed.
//
////////////////////////////////////////////////////////////////////////////////
//
// Backwards Compatibility With Original RChannel/RChannel1 Implimentation
// -----------------------------------------------------------------------
//
// In extending the functionality of RChannel to include compression, the first
// obstacle was that the original author made a STUPID mistake by failing to
// write out a version number when saving an RChannel.  Lucky for him, it turns
// out the first thing an RChannel saves is an RString, and the first thing an
// RString saves is the string's length as a long (32-bit) value.  We can
// safely assume that an RChannel's name (which was stored in this RString)
// would never approach, never mind exceed, 65K.  Newer RChannel files are
// written with a header that is well above 65K, so if the first long in an
// RChannel file is less than 65K, we can assume it's an older file.
//
//
// Notes About RString
// -------------------
//
// RChannel currently uses an RString to hold the channel's name.  Alternatives
// that were discussed where a four-character-code and an either-character-code.
// However, the RString won out because we felt that if you were going to use
// a name, you wouldn't want to deal with the 4 or 8 character limitations,
// and in any case, the storage overhead of an RString is minimal.  The main
// thing we give up with an RString is search speed, which would be far better
// with a four- or eight-character code.
//
//
// Ideas About Future Additions
// ----------------------------
//
// A spline-based channel would be great for certain types of data.  We
// simply store some control points that create the spline, and then we
// the user asks for the value at a specified time, we simply do the
// math to figure out the value at that time.
//
// Another type of channel would be designed to store data that changes at
// non-regular intervals.  In some cases we might be able to use a higher
// "sampling rate" to simulate what we need, but that might add tons of extra
// data, so there really is a need for channel designed for this purpose.
//
// As usual, greater memory efficienty comes at the expense of time.  For
// instance, we can store a timestamp with each pointer to data, and these
// pointers can still reference the same data.  The memory overhead goes to
// 8 bytes per change, but we now need to do some kind of search to find the
// right data for a given time.  For a reasonable number of items, this might
// not be too bad.
//
// Standard ways of speeding up searches can be used (binary trees, hash
// tables, etc.)  Yet another method might take advantage of the fact that
// most accesses will be very close to the previous access, so if we returned
// some sort of "handle" we could use that on future accesses so we know where
// to start, making it very fast.  Unfortunately, the handle would be unique
// to this type of container, making it an icky solution.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef CHANNEL_H
#define CHANNEL_H


#include "Blue.h"
#include "ORANGE/RString/rstring.h"


// This patch allows us to compile with MIPS version of VC++ 4.0
#if defined(_M_MRX000)
	#ifndef bool
		typedef int bool;
	#endif
#endif


////////////////////////////////////////////////////////////////////////////////
//
// RChanCore class implimentation
//
////////////////////////////////////////////////////////////////////////////////
template<class datat>
class RChanCore
	{
	//------------------------------------------------------------------------------
	// Types
	//------------------------------------------------------------------------------
	public:

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		short m_sLoopFlags;										// Looping flags


	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		//
		// Constructor
		//
		////////////////////////////////////////////////////////////////////////////////
		RChanCore()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Destructor (MUST be virtual!)
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual ~RChanCore()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Reset (restores object to the state it was in after being constructed)
		// *** Derived classes MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual void Reset(void)
			{
			m_sLoopFlags = 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Load channel from current position of already-open file
		// *** Derived classes MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual short Load(										// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  RFile to load from
			short sFileVersion)									// In:  Channel file version number
			{
			// Original version (#0) didn't store loop flags, but all others do.
			if (sFileVersion > 0)
				{
				// Read data
				pFile->Read(&m_sLoopFlags);
				}
			else
				{
				// Set data to default values
				m_sLoopFlags = 0;
				}

			return pFile->Error();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Save channel to current position of already-open file
		// *** Derived classes MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual short Save(										// Returns 0 if successfull, non-zero otherwise
			RFile* pFile)											// In:  RFile to save to
			{
			// Write data
			pFile->Write(m_sLoopFlags);

			return pFile->Error();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Create the specified number of items, which in effect sets the total number
		// of items being handled by this channel.
		//
		// Some types of channels (such as a spline-based channel) do not require this
		// functionality and will safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual void CreateItems(
			long lNumItems)										// In:  Number of items
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total number of items being handled by this channel.  This is the
		// LOGICAL number of items, not the PHYSICAL number of items, which could
		// be less due to compression.
		//
		// Some types of channels do not store discrete data items and will return -1
		// to indicate that this does not apply.
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual long NumItems(void)										// Returns total number of items
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Compress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual void Compress(void)
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Decompress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual void Decompress(void)
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// When modifying data items in compressed channels, it is often necessary to
		// differentiate between "real" items and "copies" of items, so that each item
		// is only modified once.  This function will return true if the specified
		// item is "real" and false if it's merely a copy of a real item.
		//
		// Some types of channels do not store discrete data items and will return
		// false for every item.
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual bool IsRealItem(								// Returns true if "real", false otherwise
			long lNum)												// In:  Item number
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will return 0
		// to indicate that the item could not be accessed.
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual datat* GetItem(									// Returns value at specified time
			long lNum)												// In:  Item number
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will safely
		// (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual void SetItem(
			long lNum,												// In:  Item number
			const datat* pdata)									// In:  Pointer to data to be added
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the channel's "value" at the specified time.  The return value is a
		// pointer to whatever type this channel contains.
		// 
		// If the specified time is below 0 and LoopAtStart is not enabled, then the
		// returned value will be for time = 0.  If the specified time is greater than
		// or equal to the value returned by Time() and LoopAtEnd is not enabled, then
		// the returned value will be for time = Time() - 1.  If looping is enabled,
		// then the specified time is "wrapped around" as necessary to map it back into
		// the range 0 to Time() - 1, and the associated value is returned.
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual datat* GetAtTime(								// Returns value at specified time
			long lTime)												// In:  Channel time
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual long TotalTime(void)							// Returns total channel time
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual void SetTotalTime(
			long lTotalTime)										// In:  Total time
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// return a value of 0 to indicate "not applicable".
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual long Resolution(void)							// Returns timing resolution
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		virtual void SetResolution(
			long lResolution)										// In:  New resolution
			= 0;														// Abstract function!


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get looping behaviour
		//
		////////////////////////////////////////////////////////////////////////////////
		short Looping(void)										// Returns looping flags
			{
			return m_sLoopFlags;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set looping behaviour.  For the flags value specify RChannel::LoopAtStart
		// and/or RChannel::LoopAtEnd (they can be combined using binary "OR")
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetLooping(
			short sLoopFlags)
			{
			m_sLoopFlags = sLoopFlags;
			}
	};



////////////////////////////////////////////////////////////////////////////////
//
// These enums are independant of the type of data being stored.  If they are
// defined within RChannel, it becomes necessary to specify the data type
// in order to user them.  By defining them outside of RChannel, they can be
// used with any RChannel instantiation.
//
////////////////////////////////////////////////////////////////////////////////

// File ID and version
typedef enum
	{
	RChannel_FileID		= 0x4E414843,	// ASCII for "CHAN" backwards (for little-endian files)
										// For backwards compatibility, this must be > 65K.
										// See comment block above for details.
	RChannel_FileVersion	= 1				// Version number (0 is reserved for original version)
	} RChannel_FileHeader;

// We need type information primarily so that we can instantiate the correct
// class when loading channel data from a file.
typedef enum
	{
	RChannel_Nothing,
	RChannel_Array,
	RChannel_ArrayOfPtrs
	} RChannel_Types;

// Looping-related flags.
typedef enum
	{
	RChannel_LoopAtStart = 1,
	RChannel_LoopAtEnd   = 2
	} RChannel_LoopFlags;


////////////////////////////////////////////////////////////////////////////////
//
// This is a dummy channel type that is used in cases where the user hasn't
// set a specific type of channel.  This way, each RCHannel function can simply
// call the core object without first having to check if the core object exists.
// The end result is faster code.
//
////////////////////////////////////////////////////////////////////////////////
template <class datat>
class RChanCoreNothing : public RChanCore<datat>
	{
	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:


	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		//
		// Constructor
		//
		////////////////////////////////////////////////////////////////////////////////
		RChanCoreNothing()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Destructor
		//
		////////////////////////////////////////////////////////////////////////////////
		~RChanCoreNothing()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Reset (restores object to the state it was in after being constructed)
		// *** MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void)
			{
			// Call base class implimentation
			RChanCore<datat>::Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Load channel from current position of already-open file
		// *** MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  RFile to load from
			short sFileVersion)									// In:  Channel file version number
			{
			// Reset to get rid of any existing data
			Reset();

			// Call base class implimentation
			return RChanCore<datat>::Load(pFile, sFileVersion);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Save channel to current position of already-open file
		// *** MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile)											// In:  RFile to save to
			{
			// Call base class implimentation
			return RChanCore<datat>::Save(pFile);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Create the specified number of items, which in effect sets the total number
		// of items being handled by this channel.
		//
		// Some types of channels (such as a spline-based channel) do not require this
		// functionality and will safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void CreateItems(
			long lNumItems)										// In:  Number of items
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total number of items being handled by this channel.  This is the
		// LOGICAL number of items, not the PHYSICAL number of items, which could
		// be less due to compression.
		//
		// Some types of channels do not store discrete data items and will return -1
		// to indicate that this does not apply.
		//
		////////////////////////////////////////////////////////////////////////////////
		long NumItems(void)										// Returns total number of items
			{
			return -1;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Compress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void Compress(void)
			{
			// This channel doesn't support compression
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Decompress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void Decompress(void)
			{
			// This channel doesn't support compression
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// When modifying data items in compressed channels, it is often necessary to
		// differentiate between "real" items and "copies" of items, so that each item
		// is only modified once.  This function will return true if the specified
		// item is "real" and false if it's merely a copy of a real item.
		//
		// Some types of channels do not store discrete data items and will return
		// false for every item.
		//
		////////////////////////////////////////////////////////////////////////////////
		bool IsRealItem(											// Returns true if "real", false otherwise
			long lNum)												// In:  Item number
			{
			return false;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will return 0
		// to indicate that the item could not be accessed.
		//
		////////////////////////////////////////////////////////////////////////////////
		datat* GetItem(											// Returns value at specified time
			long lNum)												// In:  Item number
			{
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will safely
		// (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetItem(
			long lNum,												// In:  Item number
			const datat* pdata)									// In:  Pointer to data to be added
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the channel's "value" at the specified time.  The return value is a
		// pointer to whatever type this channel contains.  It must be returned as a
		// void* because this is a virtual function, and the base class obviously can't
		// declare it as returning any particular type of data.
		// 
		// If the specified time is below 0 and LoopAtStart is not enabled, then the
		// returned value will be for time = 0.  If the specified time is greater than
		// or equal to the value returned by Time() and LoopAtEnd is not enabled, then
		// the returned value will be for time = Time() - 1.  If looping is enabled,
		// then the specified time is "wrapped around" as necessary to map it back into
		// the range 0 to Time() - 1, and the associated value is returned.
		//
		////////////////////////////////////////////////////////////////////////////////
		datat* GetAtTime(											// Returns pointer to value
			long lTime)												// In:  Channel time
			{
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		long TotalTime(void)
			{
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetTotalTime(
			long lTotalTime)										// In:  Total time
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// return a value of 0 to indicate "not applicable".
		//
		////////////////////////////////////////////////////////////////////////////////
		long Resolution(void)
			{
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetResolution(
			long lResolution)										// In:  New resolution
			{
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// This type of channel is designed for fixed-size data which is generated at
// fixed time intervals and which tends to change often enough that the
// cummulative overhead of storing "repeats" is not unreasonable.
//
// Federal Consumer Guidelines:
//		Memory Overhead:	12 bytes per channel, 0 bytes per data item
//		Time Complexity:	1 compare + 1 divide + 1 add
//
////////////////////////////////////////////////////////////////////////////////
template <class datat>
class RChanCoreArray : public RChanCore<datat>
	{
	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		long m_lTotalTime;										// Total time
		long m_lInterval;											// Time interval (ex: at 20 fps this would be 50)
		long m_lNumItems;											// Number of data items
		datat* m_pArray;											// Pointer to array of elements


	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		//
		// Constructor
		//
		////////////////////////////////////////////////////////////////////////////////
		RChanCoreArray()
			{
			m_pArray = 0;
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Destructor
		//
		////////////////////////////////////////////////////////////////////////////////
		~RChanCoreArray()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Reset (restores object to the state it was in after being constructed)
		// *** MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void)
			{
			Free();
			m_lTotalTime = 0;
			m_lInterval = 1;	// to avoid divide-by-zero errors
			m_lNumItems = 0;

			// Call base class implimentation
			RChanCore<datat>::Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Load channel from current position of already-open file
		// *** MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  RFile to load from
			short sFileVersion)									// In:  Channel file version number
			{
			short sResult = 0;
			
			// Reset to get rid of any existing data
			Reset();

			// Call base class implimentation
			sResult = RChanCore<datat>::Load(pFile, sFileVersion);
			if (sResult == 0)
				{
				// Read a bunch 'o stuff
				pFile->Read(&m_lTotalTime);
				pFile->Read(&m_lInterval);
				pFile->Read(&m_lNumItems);
				if (!pFile->Error() && (m_lNumItems > 0))
					{
					// Create array of items
					m_pArray = new datat[m_lNumItems];
					ASSERT(m_pArray != 0);

					// Let each item load itself
					for (long l = 0; (l < m_lNumItems) && (sResult == 0); l++)
						sResult = rspAnyLoad(&(m_pArray[l]), pFile);
					}
				else
					{
					sResult = -1;
					TRACE("RChanCoreArray::Load(): Error reading from file!\n");
					}
				}

			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Save channel to current position of already-open file
		// *** MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile)											// In:  RFile to save to
			{
			short sResult = 0;

			// Call base class implimentation
			sResult = RChanCore<datat>::Save(pFile);
			if (sResult == 0)
				{
				// Save a bunch 'o stuff
				pFile->Write(&m_lTotalTime);
				pFile->Write(&m_lInterval);
				pFile->Write(&m_lNumItems);

				// Let each item save itself
				for (long l = 0; (l < m_lNumItems) && (sResult == 0); l++)
					sResult = rspAnySave(&(m_pArray[l]), pFile);
				}

			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Create the specified number of items, which in effect sets the total number
		// of items being handled by this channel.
		//
		// Some types of channels (such as a spline-based channel) do not require this
		// functionality and will safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void CreateItems(
			long lNumItems)										// In:  Number of items
			{
			Alloc(lNumItems);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total number of items being handled by this channel.  This is the
		// LOGICAL number of items, not the PHYSICAL number of items, which could
		// be less due to compression.
		//
		// Some types of channels do not store discrete data items and will return -1
		// to indicate that this does not apply.
		//
		////////////////////////////////////////////////////////////////////////////////
		long NumItems(void)										// Returns total number of items
			{
			return m_lNumItems;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Compress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void Compress(void)
			{
			// This channel doesn't support compression
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Decompress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void Decompress(void)
			{
			// This channel doesn't support compression
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// When modifying data items in compressed channels, it is often necessary to
		// differentiate between "real" items and "copies" of items, so that each item
		// is only modified once.  This function will return true if the specified
		// item is "real" and false if it's merely a copy of a real item.
		//
		// Some types of channels do not store discrete data items and will return
		// false for every item.
		//
		////////////////////////////////////////////////////////////////////////////////
		bool IsRealItem(											// Returns true if "real", false otherwise
			long lNum)												// In:  Item number
			{
			// Every item is real
			return true;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will return 0
		// to indicate that the item could not be accessed.
		//
		////////////////////////////////////////////////////////////////////////////////
		datat* GetItem(											// Returns value at specified time
			long lNum)												// In:  Item number
			{
			ASSERT(lNum < m_lNumItems);
			return m_pArray + lNum;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will safely
		// (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetItem(
			long lNum,												// In:  Item number
			const datat* pdata)									// In:  Pointer to data to be added
			{
			ASSERT(lNum < m_lNumItems);
			m_pArray[lNum] = *pdata;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the channel's "value" at the specified time.  The return value is a
		// pointer to whatever type this channel contains.  It must be returned as a
		// void* because this is a virtual function, and the base class obviously can't
		// declare it as returning any particular type of data.
		// 
		// If the specified time is below 0 and LoopAtStart is not enabled, then the
		// returned value will be for time = 0.  If the specified time is greater than
		// or equal to the value returned by Time() and LoopAtEnd is not enabled, then
		// the returned value will be for time = Time() - 1.  If looping is enabled,
		// then the specified time is "wrapped around" as necessary to map it back into
		// the range 0 to Time() - 1, and the associated value is returned.
		//
		////////////////////////////////////////////////////////////////////////////////
		datat* GetAtTime(											// Returns pointer to value
			long lTime)												// In:  Channel time
			{
			if (lTime >= m_lTotalTime)
				{
				if (this->m_sLoopFlags & RChannel_LoopAtEnd)
					lTime %= m_lTotalTime;
				else
					lTime = m_lTotalTime - 1;
				}
			else if (lTime < 0)
				{
				if (this->m_sLoopFlags & RChannel_LoopAtStart)
					lTime = (m_lTotalTime - 1) - ((-1 - lTime) % m_lTotalTime);
				else
					lTime = 0;
				}
			return m_pArray + (lTime / m_lInterval);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		long TotalTime(void)
			{
			return m_lTotalTime;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetTotalTime(
			long lTotalTime)										// In:  Total time
			{
			m_lTotalTime = lTotalTime;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// return a value of 0 to indicate "not applicable".
		//
		////////////////////////////////////////////////////////////////////////////////
		long Resolution(void)
			{
			return m_lInterval;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetResolution(
			long lResolution)										// In:  New resolution
			{
			m_lInterval = lResolution;
			}


	protected:
		////////////////////////////////////////////////////////////////////////////////
		//
		// Allocate array
		//
		////////////////////////////////////////////////////////////////////////////////
		void Alloc(long lNum)
			{
			Free();
			m_pArray = new datat[lNum];
			ASSERT(m_pArray != 0);
			m_lNumItems = lNum;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Free array
		//
		////////////////////////////////////////////////////////////////////////////////
		void Free(void)
			{
			delete []m_pArray;
			m_pArray = 0;
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// This type of channel is designed for data that is generated at fixed time
// intervals and may contain repeats.
//
// Federal Consumer Guidelines:
//		Memory Overhead:	12 bytes per channel, 4 bytes per data item
//		Time Complexity:	1 compare + 1 divide + 1 add + 1 dereference
//
////////////////////////////////////////////////////////////////////////////////
template <class datat>
class RChanCoreArrayOfPtrs: public RChanCore<datat>
	{
	//------------------------------------------------------------------------------
	// Important implimentation note!
	//
	// To allow the data items to be compressed, we use an array of pointers to
	// data items, instead of an array of data items.  When Compress() is called,
	// we compare all the data items, and for each pair of duplicate items, we
	// get rid of one copy of the item and point both pointers at the remaining
	// item.  This process continues until all the items have been compared to
	// one another.  In the best possible case, all the pointers will refer to a
	// single data item, although that obviously depends on the data items.
	//
	// When compressing the items, we always start at the beginning of the array
	// and work towards the end.  Whenever we find duplicate items, we always
	// keep the item closest to the front of the list and get rid of the other one.
	// It is very important that we do it this way because when we save the channel,
	// we need some way to indicate which pointers are referring to "real" items
	// and which pointers are referring to items "owned" by other pointers.  If
	// pointers always refer to items that occur earlier in the list, then it makes
	// loading the data very simple.  If this weren't the case, a whole new method
	// would have to be devised.
	//------------------------------------------------------------------------------


	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		long m_lTotalTime;										// Total time
		long m_lInterval;											// Time interval (ex: at 20 fps this would be 50)
		long m_lNumItems;											// Logical number of data items
		long m_lRealNumItems;									// Actual number of data items, which may be less
																		// than m_lNumItems if items were compressed.
		datat** m_pArrayOfPtrs;									// Pointer to array of pointers to data items.
																		// This extra level of indirection makes it easy
																		// to compress data items by allowing more than
																		// one pointer to refer to the same data item.
		U8* m_pArrayOfFlags;										// Pointer to array of flags.  If flag is 0, it
																		// means it's pointing at it's own data item.  If
																		// flag is 1, it means it's pointing at another
																		// item that matched the original item -- in other
																		// words, the item was "compressed" out.  This is
																		// primarily itended for internal use since users
																		// of the channel don't need to know which pointers
																		// are "real" and which are "compressed".

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		//
		// Constructor
		//
		////////////////////////////////////////////////////////////////////////////////
		RChanCoreArrayOfPtrs()
			{
			m_pArrayOfPtrs = 0;
			m_pArrayOfFlags = 0;
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Destructor
		//
		////////////////////////////////////////////////////////////////////////////////
		~RChanCoreArrayOfPtrs()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Reset (restores object to the state it was in after being constructed)
		// *** MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void)
			{
			Free();
			m_lTotalTime = 0;
			m_lInterval = 1;	// to avoid divide-by-zero errors
			m_lNumItems = 0;
			m_lRealNumItems = 0;

			// Call base class implimentation
			RChanCore<datat>::Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Load channel from current position of already-open file
		// *** MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  RFile to load from
			short sFileVersion)									// In:  Channel file version number
			{
			short sResult = 0;
			
			// Reset to get rid of any existing data
			Reset();

			// Call base class implimentation
			sResult = RChanCore<datat>::Load(pFile, sFileVersion);
			if (sResult == 0)
				{
				// Read a bunch 'o stuff
				pFile->Read(&m_lTotalTime);
				pFile->Read(&m_lInterval);
				pFile->Read(&m_lNumItems);
				if (!pFile->Error() && (m_lNumItems > 0))
					{
					// Create array of pointers to data items
					m_pArrayOfPtrs = new datat*[m_lNumItems];
					ASSERT(m_pArrayOfPtrs);

					// Create array of flags
					m_pArrayOfFlags = new U8[m_lNumItems];
					ASSERT(m_pArrayOfFlags);

					// The file contains a value corresponding to each of our pointers.
					// If the value is -1, it means we should create a new data item,
					// set the pointer to point at it, and then tell it to load itself.
					// If the value is anything else, it is treated as an index into
					// the array of pointers, and we set our pointer equal to whatever
					// pointer that index specifies.  This way, multiple pointers can
					// refer to the same data item, which is how this data is compressed.
					m_lRealNumItems = 0;
					for (long l = 0; (l < m_lNumItems) && (sResult == 0); l++)
						{
						long lValue;
						if (pFile->Read(&lValue) == 1)
							{
							if (lValue == -1)
								{
								m_pArrayOfPtrs[l] = new datat;
								m_pArrayOfFlags[l] = 0;
								sResult = rspAnyLoad(m_pArrayOfPtrs[l], pFile);
								m_lRealNumItems++;
								}
							else
								{
								if (lValue < l)
									{
									m_pArrayOfPtrs[l] = m_pArrayOfPtrs[lValue];
									m_pArrayOfFlags[l] = 1;
									}
								else
									{
									TRACE("RChanCoreArrayOfPtrs::Load(): Internal error: Reference to item that hasn't been loaded yet!\n");
									sResult = -1;
									}
								}
							}
						else
							{
							TRACE("RChanCoreArrayOfPtrs::Load(): Error reading value from file!\n");
							sResult = -1;
							}
						}
					}
				else
					{
					sResult = -1;
					TRACE("RChanCoreArrayOfPtrs::Load(): Error reading from file!\n");
					}
				}

			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Save channel to current position of already-open file
		// *** MUST call base class implimentation ***
		//
		////////////////////////////////////////////////////////////////////////////////
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile)											// In:  RFile to save to
			{
			short sResult = 0;

			// Call base class implimentation
			sResult = RChanCore<datat>::Save(pFile);
			if (sResult == 0)
				{
				// Save a bunch 'o stuff
				pFile->Write(&m_lTotalTime);
				pFile->Write(&m_lInterval);
				pFile->Write(&m_lNumItems);

				// For each pointer, we save a value that will indicate to the Load()
				// function how to handle that pointer.  For pointers whose compress
				// flag is 0, we write out a -1 followed by whatever data the item
				// itself wants to save to the file.  For pointers whose compress flag
				// is 1, we figure out the index of the pointer that actually "owns"
				// the data item we're pointing at, and we write that index.  Of course,
				// in that case, we don't write the data item itself since it was already
				// saved ealier.
				for (long l = 0; (l < m_lNumItems) && (sResult == 0); l++)
					{
					if (m_pArrayOfFlags[l] == 0)
						{
						// Write -1 followed by data item
						pFile->Write((long)-1);
						sResult = pFile->Error();
						if (sResult == 0)
							sResult = rspAnySave(m_pArrayOfPtrs[l], pFile);
						}
					else
						{
						// Find index of pointer whose data we're pointing to
						long j;
						for (j = 0; j < l; j++)
							{
							if (m_pArrayOfPtrs[l] == m_pArrayOfPtrs[j])
								break;
							}
						if (j < l)
							{
							// Write index
							pFile->Write(j);
							sResult = pFile->Error();
							}
						else
							{
							TRACE("RChanCoreArrayOfPtrs::Save(): Internal error: Couldn't find matching pointer!\n");
							sResult = -1;
							}
						}
					}
				}

			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Create the specified number of items, which in effect sets the total number
		// of items being handled by this channel.
		//
		// Some types of channels (such as a spline-based channel) do not require this
		// functionality and will safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void CreateItems(
			long lNumItems)										// In:  Number of items
			{
			Alloc(lNumItems);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total number of items being handled by this channel.  This is the
		// LOGICAL number of items, not the PHYSICAL number of items, which could
		// be less due to compression.
		//
		// Some types of channels do not store discrete data items and will return -1
		// to indicate that this does not apply.
		//
		////////////////////////////////////////////////////////////////////////////////
		long NumItems(void)										// Returns total number of items
			{
			return m_lNumItems;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Compress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void Compress(void)
			{
			//
			// Compress the data in this channel by removing all duplicate data items.
			//
			// This function utilizes the '==' operation to determine whether two data items
			// are the same or not.  Any data type that utilizes this type of channel must
			// therefore support the '==' operator.
			//

			// Go through all the items, comparing them to all the other items that
			// aren't already compressed.  If two such items are the same, then change
			// the second item's pointer to point at the first item, then delete the
			// second item and mark it as "compressed".
			for (long i = 0; i < m_lNumItems; i++)
				{
				// Only if this isn't already compressed
				if (m_pArrayOfFlags[i] == 0)
					{
					for (long j = i + 1; j < m_lNumItems; j++)
						{
						// Only if this isn't already compressed
						if (m_pArrayOfFlags[j] == 0)
							{
							// Compare items
							if (*m_pArrayOfPtrs[i] == *m_pArrayOfPtrs[j])
								{
								delete m_pArrayOfPtrs[j];
								m_pArrayOfPtrs[j] = m_pArrayOfPtrs[i];
								m_pArrayOfFlags[j] = 1;

								// Adjust real number of items
								m_lRealNumItems--;
								}
							}
						}
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Decompress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void Decompress(void)
			{
			//
			// Decompress the data in this channel by duplicating any data items that
			// are currently being compressed.
			//
			// This function utilizes an assignment constructor to duplicate data items.
			// Any data type that utilizes this type of channel must therefore support the
			// assignment operator.
			//
			for (long i = 0; i < m_lNumItems; i++)
				{
				if (m_pArrayOfFlags[i] == 1)
					{
					// Create a copy of this item using the item's assignment constructor
					m_pArrayOfPtrs[i] = new datat(*m_pArrayOfPtrs[i]);
					m_pArrayOfFlags[i] = 0;

					// Adjust real number of items
					m_lRealNumItems++;
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// When modifying data items in compressed channels, it is often necessary to
		// differentiate between "real" items and "copies" of items, so that each item
		// is only modified once.  This function will return true if the specified
		// item is "real" and false if it's merely a copy of a real item.
		//
		// Some types of channels do not store discrete data items and will return
		// false for every item.
		//
		////////////////////////////////////////////////////////////////////////////////
		bool IsRealItem(											// Returns true if "real", false otherwise
			long lNum)												// In:  Item number
			{
			// If flag is 0 then item is real
			if (m_pArrayOfFlags[lNum] == 0)
				return true;
			// Otherwise item if a copy
			return false;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will return 0
		// to indicate that the item could not be accessed.
		//
		////////////////////////////////////////////////////////////////////////////////
		datat* GetItem(											// Returns value at specified time
			long lNum)												// In:  Item number
			{
			ASSERT(lNum < m_lNumItems);
			return m_pArrayOfPtrs[lNum];
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will safely
		// (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetItem(
			long lNum,												// In:  Item number
			const datat* pdata)									// In:  Pointer to data to be added
			{
			ASSERT(lNum < m_lNumItems);
			*m_pArrayOfPtrs[lNum] = *pdata;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the channel's "value" at the specified time.  The return value is a
		// pointer to whatever type this channel contains.  It must be returned as a
		// void* because this is a virtual function, and the base class obviously can't
		// declare it as returning any particular type of data.
		// 
		// If the specified time is below 0 and LoopAtStart is not enabled, then the
		// returned value will be for time = 0.  If the specified time is greater than
		// or equal to the value returned by Time() and LoopAtEnd is not enabled, then
		// the returned value will be for time = Time() - 1.  If looping is enabled,
		// then the specified time is "wrapped around" as necessary to map it back into
		// the range 0 to Time() - 1, and the associated value is returned.
		//
		////////////////////////////////////////////////////////////////////////////////
		datat* GetAtTime(											// Returns pointer to value
			long lTime)												// In:  Channel time
			{
			if (lTime >= m_lTotalTime)
				{
				if (this->m_sLoopFlags & RChannel_LoopAtEnd)
					lTime %= m_lTotalTime;
				else
					lTime = m_lTotalTime - 1;
				}
			else if (lTime < 0)
				{
				if (this->m_sLoopFlags & RChannel_LoopAtStart)
					lTime = (m_lTotalTime - 1) - ((-1 - lTime) % m_lTotalTime);
				else
					lTime = 0;
				}
			return m_pArrayOfPtrs[lTime / m_lInterval];
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		long TotalTime(void)
			{
			return m_lTotalTime;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetTotalTime(
			long lTotalTime)										// In:  Total time
			{
			m_lTotalTime = lTotalTime;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// return a value of 0 to indicate "not applicable".
		//
		////////////////////////////////////////////////////////////////////////////////
		long Resolution(void)
			{
			return m_lInterval;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetResolution(
			long lResolution)										// In:  New resolution
			{
			m_lInterval = lResolution;
			}


	protected:
		////////////////////////////////////////////////////////////////////////////////
		//
		// Allocate array
		//
		////////////////////////////////////////////////////////////////////////////////
		void Alloc(long lNum)
			{
			Free();

			m_lNumItems = lNum;
			m_lRealNumItems = lNum;

			// Create array of pointers to data items
			m_pArrayOfPtrs = new datat*[m_lNumItems];
			ASSERT(m_pArrayOfPtrs);

			// Create array of flags
			m_pArrayOfFlags = new U8[m_lNumItems];
			ASSERT(m_pArrayOfFlags);

			// Create actual data items and mark each item as uncompressed
			for(long l = 0; l < m_lNumItems; l++)
				{
				m_pArrayOfPtrs[l] = new datat;
				ASSERT(m_pArrayOfPtrs[l]);
				m_pArrayOfFlags[l] = 0;
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Free array
		//
		////////////////////////////////////////////////////////////////////////////////
		void Free(void)
			{
			// Delete actual data items.  If an item is marked as compressed, it means
			// that the item being pointed at is not "owned" by this pointer, so we
			// don't actually delete it (the pointer that owns the data will delete it).
			if (m_pArrayOfPtrs)
				{
				for(long l = 0; l < m_lNumItems; l++)
					{
					if (m_pArrayOfFlags[l] == 0)
						delete m_pArrayOfPtrs[l];
					m_pArrayOfPtrs[l] = 0;
					}
				}

			// Delete array of pointers to data items
			delete []m_pArrayOfPtrs;
			m_pArrayOfPtrs = 0;

			// Delete array of flags
			delete []m_pArrayOfFlags;
			m_pArrayOfFlags = 0;
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// RChannel class implimentation
//
////////////////////////////////////////////////////////////////////////////////
template <class datat>
class RChannel
	{
	//------------------------------------------------------------------------------
	// Types
	//------------------------------------------------------------------------------
	public:


	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		short m_sFileVersion;									// File version number
		RChannel_Types m_type;									// Channel type
		RString m_strName;										// Channel name
		RChanCore<datat>* m_pcore;								// Core channel object


	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		//
		// Default constructor
		//
		////////////////////////////////////////////////////////////////////////////////
		RChannel(void)
			{
			m_sFileVersion = RChannel_FileVersion;	// Default to latest version
			m_type = RChannel_Nothing;						// Default to Nothing type
			m_strName.Clear();							// Clear name
			m_pcore = ConstructCore(m_type);			// Create core
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Alternate constructor allows choosing channel type
		//
		////////////////////////////////////////////////////////////////////////////////
		RChannel(
			RChannel_Types type)
			{
			m_sFileVersion = RChannel_FileVersion;	// Default to latest version
			m_type = type;									// Set type
			m_strName.Clear();							// Clear name
			m_pcore = ConstructCore(m_type);			// Create core
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Destructor
		//
		////////////////////////////////////////////////////////////////////////////////
		~RChannel()
			{
			Reset();
			// If not NULL, this is safe.
			delete m_pcore;
			// No need to set to NULL, this object is done.
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Reset (restores object to the state it was in after being constructed)
		//
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void)
			{
			m_sFileVersion = RChannel_FileVersion;
			m_strName.Clear();
			m_pcore->Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get channel type
		//
		////////////////////////////////////////////////////////////////////////////////
		RChannel_Types Type(void)								// Returns channel type
			{
			return m_type;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set channel type.
		//
		// WARNING: Any existing data items will be destroyed!
		//
		// In the future, a conversion function may be supplied to convert from one
		// channel type to another while preserving the data items.
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetType(
			RChannel_Types type)									// In:  New type
			{
			// If the type is the same as the current type, there's no need to waste time
			// and fragment memory -- we can simply reset the existing object.  If the
			// type is different, we destroy the existing object and create a new one.
			if (m_type == type)
				{
				m_pcore->Reset();
				}
			else
				{
				delete m_pcore;
				m_type = type;
				m_pcore = ConstructCore(m_type);
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get channel name (could return NULL if no name has been assigned)
		//
		////////////////////////////////////////////////////////////////////////////////
		const char* Name(void)									// Returns pointer to channel's name
			{
			return m_strName;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set channel name
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetName(
			const char* pszName)									// In:  Name
			{
			m_strName = pszName;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Load channel from current position of already-open file
		//
		////////////////////////////////////////////////////////////////////////////////
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile)											// In:  RFile to load from
			{
			short sResult = 0;

			// Reset to get rid of any existing data
			Reset();

			U32 u32ID;
			if (pFile->Read(&u32ID) == 1)
				{
				// The original versions of RChannel did NOT use an ID or version number.
				// However, since the first thing that was written to the file was the
				// RString, and the first thing an RString wrote (at that time, anyway)
				// was the string's length as a long, we can use that to differentiate
				// older versions from newer versions.  Of course, we might be tricked
				// into thinking it's an older version when it isn't actually ANY TYPE
				// of RChannel, but that's the way it was back then, so there isn't any
				// thing we can do about it now.
				if (u32ID == RChannel_FileID)
					{
					// It's the newer file format, so get the exact version number
					if (pFile->Read(&m_sFileVersion) == 1)
						{
						// Check if we support this version
						if (m_sFileVersion == RChannel_FileVersion)
							{

							// Delete existing core
							delete m_pcore;

							// Get the channel type
							short type;
							pFile->Read(&type);
							m_type = (RChannel_Types)type;

							// Create specified core type
							m_pcore = ConstructCore(m_type);

							// Read data common to all channel types
							sResult = m_strName.Load(pFile);

							// Let the core load the rest of the file
							if (sResult == 0)
								sResult = m_pcore->Load(pFile, m_sFileVersion);
							}
						else
							{
							TRACE("RChannel::Load(): Unsupported version number!\n");
							sResult = -1;
							}
						}
					else
						{
						TRACE("RChannel::Load(): Couldn't read file version!\n");
						sResult = -1;
						}
					}
				else
					{
					//--------------------------------------------------------------------------
					// Backwards compatibility for original file format.  At that time, the
					// entire RChannel architecture was different, and the only implimented
					// type of channel was the simple array, so it's fairly easy to support.
					//--------------------------------------------------------------------------

					// We're now assuming it's the older version of the RChannel file format.
					m_sFileVersion = 0;

					// We seek back 4 bytes to "undo" the reading of 4 bytes performed above.
					pFile->Seek(-4, SEEK_CUR);

					// Load the RString
					sResult = m_strName.Load(pFile);

					// Delete existing core
					delete m_pcore;

					// Create array type (it's the only type that existed back then)
					m_type = RChannel_Array;
					m_pcore = ConstructCore(m_type);

					// Let the core load the rest of the file
					if (sResult == 0)
						sResult = m_pcore->Load(pFile, m_sFileVersion);
					}
				}
			else
				{
				TRACE("RChannel::Load(): Couldn't read file ID!\n");
				sResult = -1;
				}

			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Save channel to current position of already-open file
		//
		////////////////////////////////////////////////////////////////////////////////
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile)											// In:  RFile to save to
			{
			short sResult = 0;

			// Write header (ID and version)
			pFile->Write((U32)RChannel_FileID);
			pFile->Write((short)RChannel_FileVersion);

			// Write RChannel type
			pFile->Write((short)m_type);

			sResult = pFile->Error();
			if (sResult == 0)
				{
				// Write data common to all channel types
				sResult = m_strName.Save(pFile);
				if (sResult == 0)
					{
					// Let the core save the rest of the file
					sResult = m_pcore->Save(pFile);
					}
				}

			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Create the specified number of items, which in effect sets the total number
		// of items being handled by this channel.
		//
		// Some types of channels (such as a spline-based channel) do not require this
		// functionality and will safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void CreateItems(
			long lNumItems)										// In:  Number of items
			{
			m_pcore->CreateItems(lNumItems);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total number of items being handled by this channel.  This is the
		// LOGICAL number of items, not the PHYSICAL number of items, which could
		// be less due to compression.
		//
		// Some types of channels do not store discrete data items and will return -1
		// to indicate that this does not apply.
		//
		////////////////////////////////////////////////////////////////////////////////
		long NumItems(void)										// Returns total number of items
			{
			return m_pcore->NumItems();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Compress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void Compress(void)
			{
			m_pcore->Compress();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Decompress data stored in channel.
		//
		// Some types of channels do not support compression and will safely (and
		// silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void Decompress(void)
			{
			m_pcore->Decompress();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// When modifying data items in compressed channels, it is often necessary to
		// differentiate between "real" items and "copies" of items, so that each item
		// is only modified once.  This function will return true if the specified
		// item is "real" and false if it's merely a copy of a real item.
		//
		// Some types of channels do not store discrete data items and will return
		// false for every item.
		//
		////////////////////////////////////////////////////////////////////////////////
		bool IsRealItem(											// Returns true if "real", false otherwise
			long lNum)												// In:  Item number
			{
			return m_pcore->IsRealItem(lNum);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will return 0
		// to indicate that the item could not be accessed.
		//
		////////////////////////////////////////////////////////////////////////////////
		datat* GetItem(											// Returns value at specified time
			long lNum)												// In:  Item number
			{
			return m_pcore->GetItem(lNum);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set specified item number from the channel.
		//
		// Channels are typically accessed based on time, but this function is usefull
		// when the channel is first being created.  Looping flags have no affect on
		// this function -- the specified item number is ASSUMED TO BE VALID!
		//
		// Some types of channels do not store discrete data items and will safely
		// (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetItem(
			long lNum,												// In:  Item number
			const datat* pdata)									// In:  Pointer to data to be added
			{
			m_pcore->SetItem(lNum, pdata);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the channel's "value" at the specified time.  The return value is a
		// pointer to whatever type this channel contains.
		// 
		// If the specified time is below 0 and LoopAtStart is not enabled, then the
		// returned value will be for time = 0.  If the specified time is greater than
		// or equal to the value returned by Time() and LoopAtEnd is not enabled, then
		// the returned value will be for time = Time() - 1.  If looping is enabled,
		// then the specified time is "wrapped around" as necessary to map it back into
		// the range 0 to Time() - 1, and the associated value is returned.
		//
		////////////////////////////////////////////////////////////////////////////////
		datat* GetAtTime(											// Returns value at specified time
			long lTime)												// In:  Channel time
			{
			return m_pcore->GetAtTime(lTime);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		long TotalTime(void)										// Returns total channel time
			{
			return m_pcore->TotalTime();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set the total amount of time (ms) represented by this channel
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetTotalTime(
			long lTotalTime)										// In:  Total time
			{
			m_pcore->SetTotalTime(lTotalTime);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// return a value of 0 to indicate "not applicable".
		//
		////////////////////////////////////////////////////////////////////////////////
		long Resolution(void)									// Returns timing resolution
			{
			return m_pcore->Resolution();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set channel's minimum timing resolution (ms).
		//
		// Some types of channels do not have a specific timing resolution, and will
		// safely (and silently) ignore this function.
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetResolution(
			long lResolution)										// In:  New resolution
			{
			m_pcore->SetResolution(lResolution);
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Get looping behaviour
		//
		////////////////////////////////////////////////////////////////////////////////
		short Looping(void)										// Returns looping flags
			{
			return m_pcore->Looping();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Set looping behaviour.  For the flags value specify RChannel::LoopAtStart
		// and/or RChannel::LoopAtEnd (they can be combined using binary "OR")
		//
		////////////////////////////////////////////////////////////////////////////////
		void SetLooping(
			short sLoopFlags)										// In:  New loop flags
			{
			m_pcore->SetLooping(sLoopFlags);
			}


	protected:
		////////////////////////////////////////////////////////////////////////////////
		//
		// Construct specified type of core
		//
		////////////////////////////////////////////////////////////////////////////////
		RChanCore<datat>* ConstructCore(						// Returns pointer to new core
			RChannel_Types type)									// In:  Type of core to construct
			{
			switch(type)
				{
				case RChannel_Nothing:
					return new RChanCoreNothing<datat>;
					break;

				case RChannel_Array:
					return new RChanCoreArray<datat>;
					break;

				case RChannel_ArrayOfPtrs:
					return new RChanCoreArrayOfPtrs<datat>;
					break;
				}

			TRACE("RChannel::ConstructCore(): Unsupported type (%ld), using RChanCoreNothing<datat> instead!\n", (long)type);
			return new RChanCoreNothing<datat>;
			}
	};

#endif //CHANNEL_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
