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
// netserver.h
// Project: RSPiX
//
//	History:
//		08/30/97 MJR	Started.
//
//		11/20/97	JMI	Added support for new sCoopLevels & sCoopMode flags in 
//							StartGame and SetupGame messages.
//
//		05/26/98	JMB	Added CNetServer::GetNumberOfClients()
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NETSERVER_H
#define NETSERVER_H

#include "RSPiX.h"
#include "netmsgr.h"
#include "net.h"
#include "socket.h"
#include "ORANGE/CDT/fqueue.h"


////////////////////////////////////////////////////////////////////////////////
//
// CNetServer impliments the server side of the client-server architecture.
//
////////////////////////////////////////////////////////////////////////////////
class CNetServer
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
		class CClient
			{
			//------------------------------------------------------------------------------
			// Types, enums, etc.
			//------------------------------------------------------------------------------
			public:
				// Player states.
				typedef enum
					{
					Unused,		// Becomes "Used" when connection is established

					Used,			// Becomes "Unused" if connection is terminated before JOIN_REQ is accepted
									// Becomes "Joined" if JOIN_REQ is accepted

					Joined,		// Becomes "Unused" if dropped before game starts
									// Becomes "Dropped" if dropped after game starts

					Dropped		// Becomes "Unused" when new game starts

					} State;

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

				CNetMsgr				m_msgr;									// Messenger for communicating with client
				bool					m_bLoggedIn;							// Whether client is logged in

				long					m_lLatestPingTime;					// Latest ping time
				Net::SEQ				m_seqLastDoneFrame;					// Last frame client did
				Net::SEQ				m_seqInput;
				bool					m_bSentReadyRealm;					// Whether this client sent READY_REALM msg
				bool					m_bSentDropAck;						// Whether this client sent DROP_ACK msg
				Net::SEQ				m_seqHighestDropeeInput;			// Highest input seq client got from dropee

			//------------------------------------------------------------------------------
			// Functions
			//------------------------------------------------------------------------------
			public:
				////////////////////////////////////////////////////////////////////////////////
				// Constructor
				////////////////////////////////////////////////////////////////////////////////
				CClient()
					{
					Reset();
					}


				////////////////////////////////////////////////////////////////////////////////
				// Destructor
				////////////////////////////////////////////////////////////////////////////////
				~CClient()
					{
					Reset();
					}

				////////////////////////////////////////////////////////////////////////////////
				// Reset to post-construction state
				////////////////////////////////////////////////////////////////////////////////
				void Reset(void)
					{
					m_state					= Unused;
					m_address.Reset();
					m_acName[0]				= 0;
					m_ucColor				= 0;
					m_ucTeam					= 0;
					m_sBandwidth			= Net::FirstBandwidth;

					m_msgr.Reset();
					m_bLoggedIn				= false;

					m_lLatestPingTime		= 0;
					m_seqLastDoneFrame	= 0;
					m_seqInput				= 0;
					m_bSentReadyRealm		= false;
					m_bSentDropAck			= false;
					}
			};

	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	public:	// MADE PUBLIC BY JMB for testing TAPI
		RSocket			m_socketListen;							// Listen socket
		RSocket			m_socketAntenna;							// Socket used to receive browse broadcasts
	protected:
		RSocket::BLOCK_CALLBACK m_callback;						// Blocking callback
		unsigned short	m_usBasePort;								// Base port number

		CClient			m_aClients[Net::MaxNumIDs];			// Array of clients
		Net::ID			m_idPrevGet;								// Client we got last msg from
		Net::ID			m_idLocalClient;							// Local client's ID (or Net::Invalid if not set)

		bool				m_bGameStarted;							// Whether game was started
		bool				m_bWaitingForRealmStatus;				// Whether we're waiting for realm status messages

		char				m_acHostName[Net::MaxHostNameSize];	// Host's name
		long				m_lHostMagic;								// Host's magic number
		bool				m_bSetupGameValid;						// Whether m_msgSetupGame is valid
		NetMsg			m_msgSetupGame;							// The latest SetupGame msg (if valid)

		FQueue<Net::ID, Net::MaxNumIDs> m_qDropIDs;			// Queue of ID's to be dropped
		Net::SEQ			m_seqHighestDoneFrame;					// Highest frame
		bool				m_bWaitingForInputData;					// Whether we're waiting for input data

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CNetServer()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CNetServer()
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Reset
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void)
			{
			m_socketListen.Close(true);
			m_socketAntenna.Close(true);
			m_callback = 0;
			m_usBasePort = 0;

			for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
				m_aClients[id].Reset();
			m_idPrevGet = 0;
			m_idLocalClient = Net::InvalidID;

			m_bGameStarted = false;
			m_bWaitingForRealmStatus = false;

			m_acHostName[0] = 0;
			m_lHostMagic = 0;
			m_bSetupGameValid = false;
			m_msgSetupGame.Reset();

			m_qDropIDs.Reset();
			m_seqHighestDoneFrame = 0;
			m_bWaitingForInputData = false;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Startup
		////////////////////////////////////////////////////////////////////////////////
		short Startup(												// Returns 0 if successfull, non-zero otherwise
			unsigned short usPort,								// In:  Server base port number
			char* pszHostName,									// In:  Host name (max size is MaxHostName!!!)
			RSocket::BLOCK_CALLBACK callback);				// In:  Blocking callback


		////////////////////////////////////////////////////////////////////////////////
		// Shutdown
		////////////////////////////////////////////////////////////////////////////////
		void Shutdown(void);


		////////////////////////////////////////////////////////////////////////////////
		// Update server.  This should be called regularly.
		////////////////////////////////////////////////////////////////////////////////
		void Update(void);


		////////////////////////////////////////////////////////////////////////////////
		// Get message
		////////////////////////////////////////////////////////////////////////////////
		void GetMsg(
			NetMsg* pmsg);											// Out: Message is returned here


	private:
		////////////////////////////////////////////////////////////////////////////////
		// Send message to specified client
		////////////////////////////////////////////////////////////////////////////////
		void SendMsg(
			Net::ID id,												// In:  ID to send message to
			NetMsg* pmsg);											// In:  Message to send


		////////////////////////////////////////////////////////////////////////////////
		// Send message to specified group of clients - only Joined clients are allowed!
		////////////////////////////////////////////////////////////////////////////////
		void SendMsg(
			U16 mask,												// In:  Bit-mask indicating who to send to
			NetMsg* pmsg);											// In:  Message to send


		////////////////////////////////////////////////////////////////////////////////
		// Send message to all joined clients
		////////////////////////////////////////////////////////////////////////////////
		void SendMsg(
			NetMsg* pmsg);											// In:  Message to send


	public:
		////////////////////////////////////////////////////////////////////////////////
		// Determine if there is more data waiting to be sent.  If there is data to
		// to be sent AND there is a send error, then that data can't be sent, so we
		// return false to indicate "no more data".
		////////////////////////////////////////////////////////////////////////////////
		bool IsMoreToSend(void);								// Returns true if more to send, false otherwise


		////////////////////////////////////////////////////////////////////////////////
		// Drop specified client
		////////////////////////////////////////////////////////////////////////////////
		void DropClient(
			Net::ID id);											// In:  Client to drop


		////////////////////////////////////////////////////////////////////////////////
		// Send the specified game settings to all joined clients
		////////////////////////////////////////////////////////////////////////////////
		void SetupGame(
			short sRealmNum,										// In:  Realm number
			const char* pszRealmFile,									// In:  Realm file name
			short sDifficulty,									// In:  Difficulty
			short sRejuvenate,									// In:  Rejuvenate flag
			short sTimeLimit,										// In:  Time limit in minutes, or negative if none
			short sKillLimit,										// In:  Kill limit, or negative if none
			short	sCoopLevels,									// In:  Zero for deathmatch levels, non-zero for cooperative levels.
			short	sCoopMode);										// In:  Zero for deathmatch mode, non-zero for cooperative mode.


		////////////////////////////////////////////////////////////////////////////////
		// Start game using specified settings
		////////////////////////////////////////////////////////////////////////////////
		void StartGame(
			Net::ID idServer,										// In:  Server's client's ID
			short sRealmNum,										// In:  Realm number
			char* pszRealmFile,									// In:  Realm file name
			short sDifficulty,									// In:  Difficulty
			short sRejuvenate,									// In:  Rejuvenate flag
			short sTimeLimit,										// In:  Time limit in minutes, or negative if none
			short sKillLimit,										// In:  Kill limit, or negative if none
			short	sCoopLevels,									// In:  Zero for deathmatch levels, non-zero for cooperative levels.
			short	sCoopMode,										// In:  Zero for deathmatch mode, non-zero for cooperative mode.
			short sFrameTime,										// In:  Time per frame (in milliseconds)
			Net::SEQ seqMaxAhead);								// In:  Initial max ahead for input versus frame seq


		////////////////////////////////////////////////////////////////////////////////
		// Abort game
		////////////////////////////////////////////////////////////////////////////////
		void AbortGame(
			unsigned char ucReason);							// In:  Why game was aborted


		////////////////////////////////////////////////////////////////////////////////
		// Tell clients to go to the next realm when they reach the specified frame seq.
		// It is okay to call this multiple times -- "extra" calls are ignored.
		//
		// The return value is true if a new "next realm" sequence was initiated, and
		// false if there was already one in progress.
		////////////////////////////////////////////////////////////////////////////////
		bool NextRealm(
			Net::SEQ seq);											// In:  The seq on which to go to next realm


		////////////////////////////////////////////////////////////////////////////////
		// Tell clients to proceed.  This is used when all players are at a point where
		// they need to be told it's time to move on.  There is NO attempt to
		// syncrhonize the clients, so this must only be used in non-critical sections
		// or at a point prior to a syncrhonized section.  For instance, if all players
		// are viewing a dialog, the local user on the server might hit a key to go
		// on, at which point this function would be called to tell all the other
		// players to proceed.
		////////////////////////////////////////////////////////////////////////////////
		void Proceed(void);


		////////////////////////////////////////////////////////////////////////////////
		// Set local client's ID
		////////////////////////////////////////////////////////////////////////////////
		void SetLocalClientID(
			Net::ID id)
			{
			m_idLocalClient = id;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get host's magic number
		////////////////////////////////////////////////////////////////////////////////
		long GetHostMagic(void)
			{
			return m_lHostMagic;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get host's name
		////////////////////////////////////////////////////////////////////////////////
		const char* GetHostName(void)
			{
			return m_acHostName;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get the specified client's name
		////////////////////////////////////////////////////////////////////////////////
		const char* GetPlayerName(
			Net::ID id)
			{
			ASSERT(id != Net::InvalidID);
			ASSERT(id < Net::MaxNumIDs);

			return m_aClients[id].m_acName;
			}

		////////////////////////////////////////////////////////////////////////////////
		//	Counts how many clients are logged in
		////////////////////////////////////////////////////////////////////////////////
		short	GetNumberOfClients()
		{
			short sNumClients = 0;
			for(int id = 0; id < Net::MaxNumIDs; id++)
				if(m_aClients[id].m_bLoggedIn)
					sNumClients++;

			return sNumClients;
		}

	protected:
		////////////////////////////////////////////////////////////////////////////////
		// Start the process of dropping a client during a game
		////////////////////////////////////////////////////////////////////////////////
		void StartDroppingClientDuringGame(
			Net::ID id);


		////////////////////////////////////////////////////////////////////////////////
		// Got all drop acks, now figure out what to do next
		//
		// A return value of true indicates that one or more clients needed inputs,
		// so the drop process is not yet complete.  A return of false indicates that
		// no clients needed inputs, so we can proceed directly to the next phase.
		////////////////////////////////////////////////////////////////////////////////
		bool GotAllDropAcks(void);


		////////////////////////////////////////////////////////////////////////////////
		// Finish the process of dropping client.  If there's more clients in the drop
		// queue, this could also start the dropping of the next victim
		////////////////////////////////////////////////////////////////////////////////
		void FinishDroppingClientDuringGame(void);
	};


#endif //NETSERVER_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
