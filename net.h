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
// net.h
// Project: Nostril (aka Postal)
//
//	History:
//		05/26/97 MJR	Started.
//
//		05/28/97	JMI	Upped NetMaxPlayers to 5.
//
//		06/16/97 MJR	Upped NetMaxPlayers to 16.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NET_H
#define NET_H


////////////////////////////////////////////////////////////////////////////////
//
// This is the main respository of hardwired network-related values
//
////////////////////////////////////////////////////////////////////////////////

// The ping code was only 3/4 implimented.  The client sent the pings every so often
// and the server echoed them back, but nobody ever made use of the ping times, and
// the server didn't check to see if a client had "timed-out" based on a lack of pings.
// So, in order to preserve bandwidth, I disabled this until (or if?) we ever need it.
#define NET_PING 0



// These macros make it easier to do comparisons between two Net::SEQ values.
// The tricky part is, of course, that they are unsigned values that may wrap
// around past 0.  These means that 65535 is considered less than 0!  In fact,
// it is exactly 1 less than 0, because if you decrement 0, you go back to 65535.
//
// The macro names are encoded as follows:
//
//			SEQ_EQ(a, b)		Returns true if "a" is equal-to "b", false otherwise
//
//			SEQ_NE(a, b)		Returns true if "a" is not-equal-to "b", false otherwise
//
//			SEQ_GT(a, b)		Returns true if "a" is greater-than "b", false otherwise
//
//			SEQ_GTE(a, b)		Returns true if "a" is greater-than-or-equal-to "b", false otherwise
//
//			SEQ_LT(a, b)		Returns true if "a" is less-than "b"
//
//			SEQ_LTE(a, b)		Returns true if "a" is less-than-or-equal-to "b"
//
#define SEQ_EQ(a, b)			((Net::SEQ)(a) == (Net::SEQ)(b))
#define SEQ_NE(a, b)			((Net::SEQ)(a) != (Net::SEQ)(b))
#define SEQ_GTE(a, b)		( (Net::SEQ)( (Net::SEQ)(a) - (Net::SEQ)(b) ) < (Net::SEQ)(Net::MaxAheadSeq * 3) )
#define SEQ_GT(a, b)			(SEQ_GTE(a, b) && SEQ_NE(a, b))
#define SEQ_LT(a, b)			SEQ_GT(b, a)
#define SEQ_LTE(a, b)		SEQ_GTE(b, a)



#ifdef WIN32
namespace Net
	{
#else
class Net
	{
	public:
#endif

	//------------------------------------------------------------------------------
	// These set some overall limits on various network stuff.
	// The primary reason for moving these into their own file was to get around
	// some inter-dependancies that came up when they were part of other files.
	//------------------------------------------------------------------------------
	typedef enum
		{
		// Maximum number of ID's.  Can't be more than 16 right now because
		// some bit masks are hardwired to 16-bits, which is 1 bit per player.
		MaxNumIDs			= 16,

		// Indicates an invalid/unused net ID
		InvalidID			= 255,

		// Maximum length of chat text
		MaxChatSize			= 80,

		// Maximum length of name text
		MaxPlayerNameSize	= 32,

		// Maximum length of realm text
		MaxRealmNameSize	= 32,

		// Maximum length of host text
		MaxHostNameSize	= 32,

		#if NET_PING
			// These values relate the a ping message that is periodically sent by the client
			// to the server.  The server echos these pings back as it receives them.  This
			// serves two purposes: (1) it let's us measure the actual ping times, which may
			// be usefull for something, and (2) if the server doesn't get a ping from a client
			// for a long time, it can decide that the client has timed-out.
			MaxTimeBetweenSends		= 1000,					// Maximum time between sends to the server
			MaxTimeBeforeTimeout		= 5000,					// Maximum time before client is considered dead
		#endif

		// Maximum amount of time to wait before giving up on an attempt to connect to the server
		MaxConnectWaitTime			= 10000,

		// This is the maximum amount of time the input sequence is allowed to get ahead of the
		// frame sequence and the minimum amount of time per frame (which can also be thought of
		// as maximum frames per second).  These two values are CRITICALLY INTER-TWINED!!!
		//
		// It is EXTREMELY IMPORTANT to consider the effect that changing these values will have
		// on MaxAheadSeq, which is the maximum amount by which the input sequence is allowed to
		// get ahead of the frame sequence.
		//
		// Consider that one player is having problems, and for whatever reason is stuck at frame 0.
		// The maximum input sequence this player will transmit to other players is 0 + MaxAheadSeq.
		// Therefore, assuming the other players receive this data, they wil AT BEST be able to
		// go up to frame 0 + MaxAheadSeq.  At that point, they will be stuck because they haven't
		// gotten any further input from the stuck player.  They will, however, send out more of
		// their own inputs, and the furthest they will get is to 0 + MaxAheadSeq + MaxAheadSeq.
		// Why?  Because they are stuck at frame 0 + MaxAheadSeq, and their inputs can get MaxAheadSeq
		// ahead of that.  So, it would seem that each player must buffer up to 2 * MaxAheadSeq worth
		// of each other player's input data.  But that's not the whole story, yet.
		//
		// Consider what happens if one of the players that was way ahead dropped from the game,
		// and if the reason the stuck player was stuck was because he never got any input from the
		// player that dropped (their communication link was only workingin in one direction.)  Now,
		// the stuck player needs to get all the inputs that he never got from one of the other
		// players, since they've already used those inputs.  At most, he will need MaxAheadSeq worth
		// of inputs, because even if other players have more than that from the dropped player, all
		// that matters is what frame they got up to -- anything beyond that was never used.  In order
		// to allow for the fact that any particular player might be the only one that got some set
		// of inputs from another player, we need each player to remember MaxAheadSeq worth of inputs
		// PRIOR to the current frame number.
		//
		// So, the final result is that each player must be able to store up to 3 * MaxAheadSeq worth
		// of inputs for each other player.
		//
		// One might be tempted to conclude separately that 3 * MaxAheadSeq could also be used to
		// determine the minimum number of bits required to store a sequence value.  In other words,
		// if 3 * MaxAheadSeq came to, say, 180, you might be tempted to say you could use an 8-bit
		// value to store the sequence numbers, and simply have them go from 0 to 255 and then wrap
		// around to 0 again -- or even have them go to 180 and then wrap-around to 0.  However,
		// one would be wrong to do this.
		//
		// The problem is that we are using datagram messages to transmit inputs between players,
		// and datagrams are unreliable.  They may not get through at all, or they may get through
		// but in a different order from the order they were sent in.  The problem as it applies to
		// the sequence numbers is that if we used a range of 0 to 180, or even 0 to 255, it is
		// quite possible that long after we went past a particular frame, we might get a very old
		// packet that contained very old data.  If we can't recognize this packet as being obsolete,
		// we would mistakingly use it's data, which would most likely throw the entire game permanently
		// out of sync.
		//
		// Therefore, while we can limit the size of the our BUFFER to 180 (using the above example),
		// we can't limit the sequence numbers to that range.  Instead, we want a much larger number
		// so we can recognize older packets and ignore them.  (Note that the opposite is not a problem --
		// we can never receive a "too new" packet, since other player's can never get too far ahead.)
		//
		// The range of sequence numbers should be determined based on how long an "old" message
		// might hang around before it finally gets delivered.  I have no idea what this time might
		// be -- 5 seconds?  10 seconds?  5 minutes?????  Consider a reasonable maximimum frame rate
		// of 30 fps (for network mode).  If we used a 16-bit sequence number, we'd get up to 65536
		// frames before wrap around.  However, in order to DETECT wrap-around, we need to use half
		// that, or 32768.  At 30fps, 32768 / 30 = 1092 seconds / 60 = 18 minutes.  This seems
		// like a very reasonable time period.  We could probably get away with less, but 16-bits
		// is convenient, and saving just a few more bits is probably not worth the hassle.
		// (By the way, the goal of using as few bits as possible is reducing the size of the
		// messages being sent back and forth).
		//
		MaxAheadOfFrameTime			= 1000,
		MinFrameTime					= 33,		// 1000/33 = 30 frames per second
		MaxAheadSeq						= MaxAheadOfFrameTime / MinFrameTime,
		TotalRequiredSeqs				= MaxAheadSeq * 3,

		// Maximum amount of time to spend receiving peer data.  If this is too low,
		// we might end up with a growing backlog of peer data.  If it is too high, we
		// could end up spending all our time getting peer data and hardly any on the game.
		// It's probably better to err on the high side because in theory the peers will be
		// sending less data than our bandwidth, so we shouldn't have to spend too much
		// time receiving it.
		MaxPeerReceiveTime		= 200,

		// Minimum time between sends to a peer.  The actual time is calculated according to the
		// available bandwidth, but even when huge bandwidths are available, we don't want to
		// go nuts saturating it.  Setting it to the minimum frame time seems like a good idea.
		MinPeerSendInterval		= MinFrameTime,

		// Datagram packet overhead that isn't under our control.  For TCP/IP, datagram
		// packets (UDP) have a 36 byte header, so that's what I went with.
		DatagramPacketHeader		= 36,

		// Maximum datagram size.  It seems that all protocols support a minimum datagram
		// size of just over 500 bytes.  Anything larger might be broken into multiple
		// datagrams, which we don't want because datagrams are not necessarily received in
		// the same order they are sent in, which is something we don't want to deal with.
		MaxDatagramSize			= 500,

		// Interval between broadcasts (clients browsing for hosts will generate a
		// "looking for a host" message every this often)
		BroadcastInterval			= 1500,

		// If we don't hear from a host again for this amount of time, then
		// we assume he isn't going to respond.  I 
		BroadcastDropTime			= BroadcastInterval * 2,

		// Add this offset to the base port to calculate the listen port
		ListenPortOffset        = 0,

		// Add this offset to the base port to calculate the broadcast port
		// (the port from which broadcasts are sent)
		BroadcastPortOffset     = 1,

		// Add this offset to the base port to calculate the antenna port
		// (the port on which broadcasts are received)
		AntennaPortOffset			= 2,

		// In order to allow for multiple games running on the same system, which
		// is only usefull for debugging, but then becomes VERY usefull, we need
		// to use a different port for each "peer".  This offset determines the
		// first peer port (add this offset onto the base port).  Each successive
		// peer uses the the next successive port.
		FirstPeerPortOffset     = 3,

		// Magic number embedded at start of broadcasts so we can validate them.
		// To avoid having to deal with endian crap, it's simply defined as four
		// bytes in a row, that are always in this order.
		BroadcastMagic0			= 0x06,
		BroadcastMagic1			= 0x16,
		BroadcastMagic2			= 0x63,
		BroadcastMagic3			= 0x89
		};

	// This should probably be unsigned, and may as well be as small as we can get
	// it since we're inherently limited to 16 players anyway.
	typedef unsigned char	ID;

	// This MUST be unsigned to work properly!!!
	// See above for an explanation of why 16-bits is a good choice.
	typedef U16					SEQ;

	//------------------------------------------------------------------------------
	// These are the various bandwidths we support
	//------------------------------------------------------------------------------
	typedef enum
		{
		FirstBandwidth = 0,

		Analog14_4 = 0,
		Analog28_8,
		Analog33_6,
		Analog57_6,
		ISDN1Channel,
		ISDN2Channel,
		LAN10Mb,
		LAN100Mb,

		// Number of entries
		NumBandwidths
		} Bandwidth;

	// In a namespace this data is extern, but in a class it's static
	#ifdef WIN32
		#define NETCRAPTHING extern
	#else
		#define NETCRAPTHING static
	#endif

	// Lookup tables associated with the NetBandwidth enums.
	NETCRAPTHING long	lBandwidthValues[Net::NumBandwidths];
	NETCRAPTHING char* BandwidthText[Net::NumBandwidths];
	};


#endif //NET_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
