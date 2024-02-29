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
// netinput.h
// Project: RSPiX
//
//	History:
//		09/02/97 MJR	Started.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NETINPUT_H
#define NETINPUT_H

#include "input.h"


////////////////////////////////////////////////////////////////////////////////
//
// CNetInput handle the buffering of inputs received from other players.
//
// The Net::SEQ values are unsigned, and last I checked were 16-bit values,
// but this same logic would work for 32-bit, too.
//
// The sequence values will increment steadly until they wrap-around.  We assume
// that the amount of time that it will take for them to wrap-around is greater
// than the amount of time a network packet can live for and still manage to get
// delivered.  With a 16-bit value at 30 frames per second, this allow for over
// 30 minutes -- I doubt a network packet can survive that long.
//
// This class impliments a "sliding window" of input values.  The window must
// be at least 3 * Net::MaxAheadSeq in size.  (A detailed explanation can be
// found in where that value is defined).
//
// Assuming Net::SEQ ranges from 0 to 65535, the window looks like this:
//
//       0-------------[ooonnnnnn]-----------------------------65535
//                         ^
//                         F
//
// The dashes represent values we don't care about -- any values that are
// NOT within the window are ignored.
//
// The "F" represents the current frame number, which determines the overall
// position of the window.  This is the LOCAL PLAYER'S frame number, not the
// frame number of the player whose data we are dealing with!
//
// As the frame number increases, the window moves to the right, eventually
// wrapping around and starting over at the left side.
//
// The window contains 3 sets of Net::MaxAheadSeq values.  The first set is
// to the left of the current frame number, and is represented by o's.  These
// are "old" values we must keep around in case this player drops from the game
// and we are the only player that received (and used!) his input values.  In
// that case, we will be asked to supply these values to the other players.
//
// The second two sets are represented by n's, and are new values we might
// have received from this player, but that we haven't yet used.
//
// Inputs may arrive out of order, which means we might get a few values,
// then get a few more values further along in the window but leaving a "hole"
// of unknown values in between.  We assume that we'll eventually get those
// values, too, but in the meantime, we can't move our frame number past that
// hole.
//
// In order to properly detect which values we did and did not get, we start
// out with the entire window filled with invalid values, and whenever we move
// the window, we mark any "newly uncovered" values as invalid, too.  As we
// receive inputs, we put them in the appropriate spots in the array,
// overwriting the invalid values in the process.  Whenever an attempt is made
// to move the frame forward, we check if the value for that frame is valid.
// If so, we can move forward.  If not, it means we haven't gotten that value
// yet, so we can't move forward yet.
//
//
// All that cool theory aside, the array is actually implimented in a somewhat
// different manner.
//
// By making it a power of 2 in size (making sure it's at least as
// large as 3 * Net::MaxAheadSeq), everything gets harder to think about, but
// very efficient.  Normally, in order to impliment a sliding window buffer,
// you need to slide around the values contained in the buffer as it moves
// forward.  By making it a power of 2 in size, we can use simple bit-masks
// to get all the indices into to the buffer to automatically wrap-around as
// needed, and suddenly we don't need to move the values anymore -- instead we
// are changing how we look at the buffer.  We are merely moving what we think
// of as the start and end of the buffer around, rather than moving everything
// within the buffer.  Kind of hard to explain, but it DOES work!
//
// Let's assume Net::SEQ is only 4-bits, so it ranges from 0 to 15.  Now let's
// say Net::MaxAheadSeq is 1, so our buffer needs to 3 times that, but instead
// we go with 4 entries, which is the next power-of-2 after 3.  The diagram
// below shows how the various input sequences will map into our buffer.  In
// order to arrive at the index into our buffer, we simply take the sequence
// number and mark out all the bits but the bottom 2, which yields a number
// between 0 and 3, and happens to be the correct index into our buffer.  The
// diagram shows how our buffer "slides along" without moving the actual
// contents -- instead, it's all just a matter of how you think of it.
//
//       0 1 2 3 4 5 6 7 8 9 A B C D E F
//       - - - - - - - - - - - - - - - -
//       0 1 2 3
//         1 2 3 0
//           2 3 0 1
//             3 0 1 2
//               0 1 2 3
//
////////////////////////////////////////////////////////////////////////////////
class CNetInput
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:
		enum
			{
			// Maximum total entries (see elsewhere for in-depth explanation)
			MaxTotalEntries	= 3 * Net::MaxAheadSeq,

			// Maxumum new entries (see elsewhere for in-depth explanation)
			MaxNewEntries     = 2 * Net::MaxAheadSeq,

			// Maximum old entries (see elsewhere for in-depth explanation)
			MaxOldEntries		= 1 * Net::MaxAheadSeq,

			// The size must be a power of two, and the mask must correspond to it.
			// Remember that this must be at LEAST as large as MaxTotalEntries!
			Size				= 256,
			Mask				= Size - 1,

			// Invalid input value
			Invalid = 0xffffffff
			};

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		UINPUT					m_aInputs[Size];						// Inputs
		U8						m_aFrameTimes[Size];					// Game time for the frames *SPA
		Net::SEQ				m_seqFrame;								// Local player's current frame number
		Net::SEQ				m_seqOldest;							// Oldest sequence we have

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CNetInput()
			{
			ASSERT(Size >= MaxTotalEntries);
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CNetInput()
			{
			Reset();
			}

		////////////////////////////////////////////////////////////////////////////////
		// Reset to post-construction state
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void)
			{
			short i = 0;
			// Clear the entire window to "invalid" values
			for (i = 0; i < Size; i++)
				m_aInputs[i] = Invalid;

			// Clear the entire window to initail values *SPA !!Eventually should input from prefs!!
			for (i = 0; i < Size; i++)
				m_aFrameTimes[i] = 100;

			// Start the frame at 0.  The oldest value always lags by a fixed distance.
			m_seqFrame = 0;
			m_seqOldest = m_seqFrame - MaxOldEntries;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Move the frame forward
		// IT IS ASSUMED THAT THE CALLER WILL NOT BE STUPID!!!
		// Don't ever move the frame forward unless the input for the current frame
		// is valid, as reported by GetInput()!
		////////////////////////////////////////////////////////////////////////////////
		void IncFrame(void)
			{
			// Make sure this makes sense!
			ASSERT(m_aInputs[m_seqFrame & Mask] != Invalid);

			// Invalidate oldest sequence
			m_aInputs[m_seqOldest & Mask] = Invalid;

			// Move forward
			m_seqFrame++;
			m_seqOldest++;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Put a new input value.
		// If the specified seq is outside of the "new value window", it is ignored.
		////////////////////////////////////////////////////////////////////////////////
		void Put(
			Net::SEQ seq,
			UINPUT input)
			{
			// If the seq falls within the "new values" window, we use it.  Note that
			// the "new values" window starts at the current frame.
			// The excessive casting is to make sure the compiler does this all as
			// unsigned math and comparisons.
			if ((Net::SEQ)(seq - m_seqFrame) < (Net::SEQ)MaxNewEntries)
				m_aInputs[seq & Mask] = input;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Put a new frame time value.  *SPA
		// If the specified seq is outside of the "new value window", it is ignored.
		////////////////////////////////////////////////////////////////////////////////
		void PutFrameTime(
			Net::SEQ seq,
			U8 frameTime)
			{
			// If the seq falls within the "new values" window, we use it.  Note that
			// the "new values" window starts at the current frame.
			// The excessive casting is to make sure the compiler does this all as
			// unsigned math and comparisons.
			if ((Net::SEQ)(seq - m_seqFrame) < (Net::SEQ)MaxNewEntries)
				{
				m_aFrameTimes[seq & Mask] = frameTime;
				}
			}

		////////////////////////////////////////////////////////////////////////////////
		// Find first invalid value starting at the specified seq and continuing to
		// the end of the "new" window.
		////////////////////////////////////////////////////////////////////////////////
		Net::SEQ FindFirstInvalid(
			Net::SEQ seq)
			{
			Net::SEQ offset = (Net::SEQ)(seq - m_seqFrame);
			while (offset < (Net::SEQ)MaxNewEntries)
				{
				if (m_aInputs[seq & Mask] == Invalid)
					break;
				seq++;
				offset++;
				}
			return seq;
			}

		// This alternative version starts at the current frame
		Net::SEQ FindFirstInvalid(void)
			{
			return FindFirstInvalid(m_seqFrame);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get the specified input value.
		// A return value of CNetInput::Invalid means that input was not available.
		////////////////////////////////////////////////////////////////////////////////
		UINPUT Get(
			Net::SEQ seq)
			{
			// If the seq falls within the total window, we get it.  Note that this
			// window starts at the oldest value, which is always Net::MaxAheadSeq less
			// than the current frame.
			// The excessive casting is to make sure the compiler does this all as
			// unsigned math and comparisons.
			if ((Net::SEQ)(seq - m_seqOldest) < (Net::SEQ)MaxTotalEntries)
				return m_aInputs[seq & Mask];
			else
				return Invalid;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get the specified frame time value. *SPA
		// A return value of CNetInput::Invalid means that input was not available.
		////////////////////////////////////////////////////////////////////////////////
		U8 GetFrameTime(
			Net::SEQ seq)
			{
			// If the seq falls within the total window, we get it.  Note that this
			// window starts at the oldest value, which is always Net::MaxAheadSeq less
			// than the current frame.
			// The excessive casting is to make sure the compiler does this all as
			// unsigned math and comparisons.
			if ((Net::SEQ)(seq - m_seqOldest) < (Net::SEQ)MaxTotalEntries)
				return m_aFrameTimes[seq & Mask];
			else
				return Invalid;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get current frame seq
		////////////////////////////////////////////////////////////////////////////////
		Net::SEQ GetFrameSeq(void)
			{
			return m_seqFrame;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get oldest frame seq
		////////////////////////////////////////////////////////////////////////////////
		Net::SEQ GetOldestSeq(void)
			{
			return m_seqOldest;
			}
	};


#endif //NETINPUT_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
