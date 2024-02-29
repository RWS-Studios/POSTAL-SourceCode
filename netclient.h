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
// netclient.h
// Project: RSPiX
//
//	History:
//		08/30/97 MJR	Started.
//
//		11/25/97	JMI	Changed m_error to m_msgError so we could store a whole
//							error message instead of just the error type.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NETCLIENT_H
#define NETCLIENT_H

#include "RSPiX.h"
#include "socket.h"
#include "net.h"
#include "netmsgr.h"
#include "NetInput.h"
#include "ORANGE/CDT/fqueue.h"
#include "IdBank.h"
#include "Average.h"

#define NUM_TIMES  8;		// Number of frame times to remember
#define TIME_MASK  0x7;	// Mask for frame sequence number to index the array of frame times
////////////////////////////////////////////////////////////////////////////////
//
// CNetClient impliments the client side of the client-server architecture.
//
////////////////////////////////////////////////////////////////////////////////
class CNetClient
	{
	//------------------------------------------------------------------------------
	// Classes
	//------------------------------------------------------------------------------
	protected:
		////////////////////////////////////////////////////////////////////////////////
		//
		// CClient is the server's representation of a client
		//
		// The general state is indicated by m_state, while the connection state is
		// indicated by m_msgr.State().  The two states will generally be "in agreement"
		// with one another, but since connecting and disconnecting are asynchronous
		// processes, it may seem like the connection status "lags behind" m_state,
		// which changes immediately.
		//
		////////////////////////////////////////////////////////////////////////////////
		class CPeer
			{
			//------------------------------------------------------------------------------
			// Types, enums, etc.
			//------------------------------------------------------------------------------
			public:
				// Player states.
				typedef enum
					{
					Unused,		// Becomes "Joined" when server sends JOINED msg

					Joined,		// Becomes "Unused" if server sends DROPPED before game starts
									// Becomes "Dropped" if server sends DROPPED after game starts

					Dropped		// Becomes "Unused" when new game starts

					} State;

				enum
					{
					NumAvgItems = 5
					};

			//------------------------------------------------------------------------------
			// Variables
			//------------------------------------------------------------------------------
			public:
				State					m_state;									// State
				RSocket::Address	m_address;								// Address
				char					m_acName[Net::MaxPlayerNameSize];// Name
				unsigned char		m_ucColor;								// Color number
				unsigned char		m_ucTeam;								// Team number
				short					m_sBandwidth;							// Net::Bandwidth

				CNetInput			m_netinput;								// Sliding window of inputs
				Net::SEQ				m_seqLastActive;						// Last active sequence (only if dropped)
				bool					m_bInactive;							// True after last active sequence was used
//				Net::SEQ				m_seqWhatHeNeeds;						// What input seq he needs from me
//				Net::SEQ				m_seqWhatINeed;						// What input seq I need from him
//				long					m_lNextSendTime;						// When to next send inputs to him
				long					m_lLastReceiveTime;					// When we last got data from him *SPA

//				FQueue<long, NumAvgItems>	m_qPings;					// Queue of ping times for running average
//				long					m_lRunnigAvgPing;						// Running average

				U16					m_idDude;								// Dude's ID

			//------------------------------------------------------------------------------
			// Functions
			//------------------------------------------------------------------------------
			public:
				////////////////////////////////////////////////////////////////////////////////
				// Constructor
				////////////////////////////////////////////////////////////////////////////////
				CPeer()
					{
					Reset();
					}


				////////////////////////////////////////////////////////////////////////////////
				// Destructor
				////////////////////////////////////////////////////////////////////////////////
				~CPeer()
					{
					Reset();
					}

				////////////////////////////////////////////////////////////////////////////////
				// Reset to post-construction state
				////////////////////////////////////////////////////////////////////////////////
				void Reset(void)
					{
					m_state						= Unused;
					m_address.Reset();
					m_acName[0]					= 0;
					m_ucColor					= 0;
					m_ucTeam						= 0;
					m_sBandwidth				= Net::FirstBandwidth;

					m_netinput.Reset();
					m_seqLastActive			= 0;
					m_bInactive					= false;
//					m_seqWhatHeNeeds			= 0;
//					m_seqWhatINeed				= 0;
//					m_lNextSendTime			= 0;
					m_lLastReceiveTime		= 0; // *SPA

//					m_qPings.Reset();
//					m_lRunnigAvgPing			= 0;

					m_idDude						= CIdBank::IdNil;
					}
			};

	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:
		// Client states
		typedef enum
			{
			Nothing,
			WaitForConnect,
			WaitForLoginResponse,
			WaitForJoinResponse,
			Joined
			} State;

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		CNetMsgr					m_msgr;							// Messenger to server
		RSocket					m_socketPeers;					// Socket used to communicate with peers
		RSocket::Address		m_addressServer;				// Server's address (with base port)
		RSocket::Address		m_addressServerListen;		// Server's address (listen port)
		RSocket::BLOCK_CALLBACK m_callback;

		State						m_state;							// My state
		NetMsg					m_msgError;						// Error type
		NetMsg::Status			m_status;						// Status type
		long						m_lTimeOut;						// Timer used to detect time-outs

		Net::ID					m_id;								// My id
		Net::ID					m_idServer;						// Server's client's ID
		short						m_sNumJoined;					// Number of joined players

		bool						m_bGameStarted;				// Whether game has started
		bool						m_bPlaying;						// true means playing, false means stopped
		bool						m_bUseHaltFrame;				// Whether to use m_seqStopFrame
		bool						m_bReachedHaltFrame;			// Whether we've reached the halt frame
		Net::SEQ					m_seqHaltFrame;				// Stop when we reach this frame seq
		Net::SEQ					m_seqInput;						// My input sequence
		Net::SEQ					m_seqFrame;						// My frame sequence
		Net::SEQ					m_seqMaxAhead;					// Max ahead for input versus frame
		Net::SEQ					m_seqInputNotYetSent;		// Input seq that we did NOT send yet
		CNetInput					m_netinput;						// My input buffer
		long						m_lFrameTime;					// Current frame time
		long						m_lNextLocalInputTime;		// When to get next local input
		bool						m_bNextRealmPending;			// Whether next realm is pending

		CPeer						m_aPeers[Net::MaxNumIDs];	// Array of peers
		/** SPA **/
		long						m_lStartTime;					// The start from which time to calculate the frame delta
		long						m_alAvgFrameTimes[8];		// Array to hold the last several average frame times
		/** SPA **/

		/** 12/16/97 AJC **/
		U16							m_u16PackageID;				// Unique number for every package sent
		/** 12/16/97 AJC **/
		bool						m_bSendNextFrame;		// 12/30/97 *SPA True if we have all the info to render the current frame (m_seqFrame)
		long						m_lMaxWaitTime;
		//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CNetClient()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CNetClient()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Reset
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void)
			{
			m_msgr.Reset();
			m_socketPeers.Reset();
			m_addressServer.Reset();
			m_addressServerListen.Reset();
			m_callback = 0;

			m_state = Nothing;
			m_msgError.msg.err.ucType	= NetMsg::ERR;
			m_msgError.msg.err.error	= NetMsg::NoError;
			m_msgError.msg.err.ulParam	= 0;

			m_status = NetMsg::NoStatus;
			m_lTimeOut = 0;

			m_id = Net::InvalidID;
			m_idServer = Net::InvalidID;
			m_sNumJoined = 0;

			m_bGameStarted = false;
			m_bPlaying = false;
			m_bUseHaltFrame = false;
			m_bReachedHaltFrame = false;
			m_seqHaltFrame = 0;
			m_seqInput = 0;
			m_seqFrame = 0;
			m_seqMaxAhead = 0;
			m_seqInputNotYetSent = 0;
			m_netinput.Reset();
			m_lFrameTime = 0;
			m_lNextLocalInputTime = 0;
			m_bNextRealmPending = false;

			for (U8 id = 0; id < Net::MaxNumIDs; id++)
				m_aPeers[id].Reset();

			/** 12/15/97 SPA **/
			m_lStartTime = 0;
			for (short i = 0; i < 8; i++)
				m_alAvgFrameTimes[i] = 100;
			/** 12/15/97 SPA **/

			/** 12/16/97 AJC **/
			m_u16PackageID = 0;
			/** 12/16/97 AJC **/

			m_bSendNextFrame = false; // 12/30/97 *SPA
			m_lMaxWaitTime = 0;

			}


		////////////////////////////////////////////////////////////////////////////////
		// Startup
		////////////////////////////////////////////////////////////////////////////////
		void Startup(
			RSocket::BLOCK_CALLBACK callback);				// In:  Blocking callback


		////////////////////////////////////////////////////////////////////////////////
		// Shutdown
		////////////////////////////////////////////////////////////////////////////////
		void Shutdown(void);


		////////////////////////////////////////////////////////////////////////////////
		// Start the process of joining the game on the specified host.  The port in
		// in the host's address is assumed to be the so-called "base port".
		//
		// If this function returns an error, it indicates that the join process has
		// failed, most likely because the currently selected protocol is not supported.
		// Even if this happens, it is still safe to call Update() and GetMsg() as if
		// no error occurred, realizing, of course, that the join process will not
		// succeed and GetMsg() will shortly return an error to that effect.
		////////////////////////////////////////////////////////////////////////////////
		short StartJoinProcess(									// Returns 0 if successfull, non-zero otherwise
			RSocket::Address* paddressHost,					// In:  Host's address
			char* pszName,											// In:  Joiner's name
			unsigned char ucColor,								// In:  Joiner's color
			unsigned char ucTeam,								// In:  Joiner's team
			short sBandwidth);									// In:  Joiner's Net::Bandwidth


		////////////////////////////////////////////////////////////////////////////////
		// Update (must be called regularly)
		////////////////////////////////////////////////////////////////////////////////
		void Update(void);


		////////////////////////////////////////////////////////////////////////////////
		// Get next available message from server
		////////////////////////////////////////////////////////////////////////////////
		void GetMsg(
			NetMsg* pmsg);											// Out: Message is returned here


		////////////////////////////////////////////////////////////////////////////////
		// Send message to server
		////////////////////////////////////////////////////////////////////////////////
		void SendMsg(
			NetMsg* pmsg,											// In:  Message to send
			bool bSendNow = true);								// In:  Whether to send now or wait until Update()


		////////////////////////////////////////////////////////////////////////////////
		// Send chat message (text is sent with player's name as a prefix)
		////////////////////////////////////////////////////////////////////////////////
		void SendChat(
			const char* pszText);								// In:  Text to send


		////////////////////////////////////////////////////////////////////////////////
		// Send text message (text is send as is)
		////////////////////////////////////////////////////////////////////////////////
		void SendText(
			const char* pszText);								// In:  Text to send


		////////////////////////////////////////////////////////////////////////////////
		// Send realm status
		////////////////////////////////////////////////////////////////////////////////
		void SendRealmStatus(
			bool bReady);


		////////////////////////////////////////////////////////////////////////////////
		// Drop self
		////////////////////////////////////////////////////////////////////////////////
		void Drop(void);


		////////////////////////////////////////////////////////////////////////////////
		// Determine if there is more data waiting to be sent.  If there is data to
		// to be sent AND there is a send error, then that data can't be sent, so we
		// return false to indicate "no more data".
		////////////////////////////////////////////////////////////////////////////////
		bool IsMoreToSend(void)
			{
			return m_msgr.IsMoreToSend();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get assigned ID
		////////////////////////////////////////////////////////////////////////////////
		Net::ID GetID(void)										// Returns ID 
			{
			return m_id;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get "servers net ID", which is really the ID of the server's client
		////////////////////////////////////////////////////////////////////////////////
		Net::ID GetServerID(void)								// Returns ID 
			{
			return m_idServer;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get the current number of players
		////////////////////////////////////////////////////////////////////////////////
		short GetNumPlayers(void)
			{
			return m_sNumJoined;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get the specified player's name
		////////////////////////////////////////////////////////////////////////////////
		const char* GetPlayerName(
			Net::ID id)
			{
			ASSERT(id != Net::InvalidID);
			ASSERT(id < Net::MaxNumIDs);

			return m_aPeers[id].m_acName;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get the specified player's color
		////////////////////////////////////////////////////////////////////////////////
		unsigned char GetPlayerColor(
			Net::ID id)
			{
			ASSERT(id != Net::InvalidID);
			ASSERT(id < Net::MaxNumIDs);

			return m_aPeers[id].m_ucColor;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine whether ths specified player needs a dude
		////////////////////////////////////////////////////////////////////////////////
		bool DoesPlayerNeedDude(
			Net::ID id)
			{
			return (m_aPeers[id].m_state == CPeer::Joined) || (m_aPeers[id].m_state == CPeer::Dropped);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get or set specified player's dude ID (not to be confused with a Net::ID)
		////////////////////////////////////////////////////////////////////////////////
		U16 GetPlayerDudeID(
			Net::ID id)
			{
			ASSERT(id != Net::InvalidID);
			ASSERT(id < Net::MaxNumIDs);

			return m_aPeers[id].m_idDude;
			}

		void SetPlayerDudeID(
			Net::ID id,
			U16 idDude)
			{
			ASSERT(id != Net::InvalidID);
			ASSERT(id < Net::MaxNumIDs);

			m_aPeers[id].m_idDude = idDude;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine whether we're playing or not
		////////////////////////////////////////////////////////////////////////////////
		bool IsPlaying(void)
			{
			return m_bPlaying;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Determine whether another frame can be done.  If so, the necessary peer
		// inputs are returned in the supplied array and the result is true.
		// Otherwise, the result is false, and a frame cannot be done.
		////////////////////////////////////////////////////////////////////////////////
		bool CanDoFrame(											// Returns true if frame can be done, false otherwise
			UINPUT aInputs[],										// Out: Total of Net::MaxNumIDs inputs returned here
			short* psFrameTime);									// Out the current frames elapsed time



		////////////////////////////////////////////////////////////////////////////////
		// Check whether local input is required
		////////////////////////////////////////////////////////////////////////////////
		bool IsLocalInputNeeded(void);


		////////////////////////////////////////////////////////////////////////////////
		// Set local input.
		// Call this if and only if IsLocalInputNeeded() returns true!
		////////////////////////////////////////////////////////////////////////////////
		void SetLocalInput(
			UINPUT input);


		////////////////////////////////////////////////////////////////////////////////
		// Get the input seq that has NOT been sent yet to any other player
		////////////////////////////////////////////////////////////////////////////////
		Net::SEQ GetInputSeqNotYetSent(void)
			{
			return m_seqInputNotYetSent;
			}


		////////////////////////////////////////////////////////////////////////////////
		// This is normally used to handle a HALT_REALM message when we get one from
		// the server.
		//
		// It is also used the local server, if there is one, to directly inform the
		// local client (that would be us) about the halt frame.  This is a necessary
		// breach of client/server separation that is more fully explained elsewhere.
		// (It may indicate a design flaw, but I'm at a loss for a better solution.)
		////////////////////////////////////////////////////////////////////////////////
		void SetHaltFrame(
			Net::SEQ seqHalt);

		////////////////////////////////////////////////////////////////////////////////
		// This is used to see if any peers have not sent data for too long of a period
		//
		// It returns the id of the first peer it finds that has exceeded the time limit
		// so that the server can drop him. It should only be called if this client is
		// attached to the server. If more than one peer has exceeded the time limit,
		// it will be handled on a subsequent pass. By then the first late peer will
		// have been marked as dropped
		// Returns -1 if no peer has exceeded time limit *SPA
		////////////////////////////////////////////////////////////////////////////////
		Net::ID CheckForLostPeer(void);

	protected:
		////////////////////////////////////////////////////////////////////////////////
		// Receive messages from peers
		////////////////////////////////////////////////////////////////////////////////
		void ReceiveFromPeers(void);


		////////////////////////////////////////////////////////////////////////////////
		// Send messages to peers
		////////////////////////////////////////////////////////////////////////////////
		void SendToPeers(void);

		////////////////////////////////////////////////////////////////////////////////
		// Send message to single peer
		////////////////////////////////////////////////////////////////////////////////
		void SendToPeer(Net::ID id, Net::SEQ seqStart, bool bSeqReq);
	};


#endif //NETCLIENT_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
