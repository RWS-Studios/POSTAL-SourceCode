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
// netmsgr.h
// Project: Nostril (aka Postal)
//
//	History:
//		05/21/97 MJR	Started.
//
//		05/25/97	JMI	Integrated CBufQ's; finished NetMsg union; and filled in
//							CNetMsgr functions.
//
//		05/25/97	JMI	Made pumping in Update() loop until conditions dictate that
//							no more data can be received/sent without blocking during
//							the current Update().
//
//		05/26/97 MJR	Moved some functions from here to .cpp file.
//							Took out "friend" stuff, made two members public instead.
//
//		05/26/97	JMI	Added 1 to NetMsgr::Chat::Len.
//
//		06/11/97	JMI	Added FINISH_REALM and FINISHED_REALM net messages.
//
//		06/14/97 MJR	Removed LOAD_REALM and FINISHED_REALM messages and
//							modified START_GAME message.
//
//		06/15/97 MJR	Added new reason for denying a join request.
//
//		08/13/97 MJR	Added difficulty to startGame message.
//					MJR	Added support for getting/putting mac addresses.
//
//		08/15/97 MJR	Fixed bug in how prototype was being read & written.
//
//		08/18/97 MJR	Added "ChangeReq" and "Changed" messages.
//							Added more parameters to JoinReq and Joined messages.
//
//		08/23/97 MJR	Added Reset().
//
//		09/01/97 MJR	Lots of changes as part of overall network overhaul.
//
//		09/02/97 MJR	Tested and tuned alot, and fixed a bunch of bugs.  Now
//							appears to be very stable.
//
//		09/07/97 MJR	Added "Proceed" message and renamed some error types.
//
//		09/09/97 MJR	Changed Connect() so it returns the error that
//							RSocket::Open() returned (if there is an error).
//
//		11/20/97	JMI	Added sCoopLevels & sCoopMode to SetupGame and 
//							StartGame.
//
//		11/24/97	JMI	Upped MinVersionNum and CurVersionNum to 0x0002 for PCs and
//							0x1002 for Macs. Also, added MacVersionBit (0x1000).
//							Also, changed VersionMismatchError to 
//							ServerVersionMismatchError and added 
//							ClientVersionMismatchError, ServerPlatformMismatchError,
//							and ClientPlatformMismatchError.
//							Also, added ulParam for a generic parameter for messages
//							(currently used by Client/ServerVersionMismatchError to
//							communicate the other machine's Postal net version number).
//
//		10/11/99	JMI	Upped net version number to 3.  This causes the version 3 
//							_client_ to reject connections from server versions 1 & 2.  
//							Client versions 1 & 2 will reject connections to version 3
//							servers.  This works this way due to issues explained in 
//							NetClient.cpp in the comment in the function Update() in
//							the case for handling LOGIN_ACCEPT when in 
//							WaitForLoginResponse state.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NETMSGR_H
#define NETMSGR_H

#include "RSPiX.h"
#include "net.h"
#include "socket.h"
#include "BufQ.h"
#include "input.h"


////////////////////////////////////////////////////////////////////////////////
// Function makes it easy to get socket address from buffer queue
////////////////////////////////////////////////////////////////////////////////
inline void GetSocketAddress(
	CBufQ* pBuf,
	RSocket::Address* paddress)
	{
	// Stuff that's common to all sockets
	long lTmp;
	pBuf->Get(&lTmp);
 	paddress->prototype = (RSocket::ProtoType)lTmp;
	pBuf->Get(&paddress->lAddressLen);

	// The rest is protocol-dependant
	switch(paddress->prototype)
		{
		case RSocket::TCPIP:
			{
			RProtocolBSDIP::AddressIP* p = (RProtocolBSDIP::AddressIP*)paddress;
			pBuf->Get(&p->address.sin_family);
			// Don't byte-swap these!!!  They are always in network order on all systems!
			pBuf->Get((U8*)&p->address.sin_port, sizeof(p->address.sin_port));
			pBuf->Get((U8*)&p->address.sin_addr, sizeof(p->address.sin_addr));
			pBuf->Get((U8*)&p->address.sin_zero, sizeof(p->address.sin_zero));
			}
			break;

		default:
			TRACE("PutSocketAddress(): Unknown protocol!\n");
			break;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Function makes it easy to put socket address to buffer queue
////////////////////////////////////////////////////////////////////////////////
inline void PutSocketAddress(
	CBufQ* pBuf,
	RSocket::Address* paddress)
	{
	// The RSocket::Address is a bit tricky, and different for each protocol
	pBuf->Put((long)paddress->prototype);
	pBuf->Put(paddress->lAddressLen);
	switch(paddress->prototype)
		{
		case RSocket::TCPIP:
				{
			RProtocolBSDIP::AddressIP* p = (RProtocolBSDIP::AddressIP*)paddress;
			pBuf->Put(p->address.sin_family);
			// Don't let these byte-swap!!!  They are always in network order on all systems!
			pBuf->Put((U8*)&p->address.sin_port, sizeof(p->address.sin_port));
			pBuf->Put((U8*)&p->address.sin_addr, sizeof(p->address.sin_addr));
			pBuf->Put((U8*)&p->address.sin_zero, sizeof(p->address.sin_zero));
			}
			break;

		default:
			TRACE("PutSocketAddress(): Unknown protocol!\n");
			break;
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Define a net message
//
////////////////////////////////////////////////////////////////////////////////
class NetMsg
	{
//	// forward declaration
//	class CNetComm;

	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:

		// Message types.  Do NOT change the order without making the same changes
		// to the CNetMsgr static array of info corersponding to these values!!!
		typedef enum
			{
			NOTHING,				// 0
			STAT,					// 1
			ERR,					// 2
			LOGIN,				// 3
			LOGIN_ACCEPT,		// 4
			LOGIN_DENY,			// 5
			LOGOUT,				// 6
			JOIN_REQ,			// 7
			JOIN_ACCEPT,		// 8
			JOIN_DENY,			// 9
			JOINED,				// 10
			CHANGE_REQ,			// 11
			CHANGED,				// 12
			DROP_REQ,			// 13
			DROPPED,				// 14
			DROP_ACK,			// 15
			INPUT_REQ,			// 16
			INPUT_DATA,			// 17
			INPUT_MARK,			// 18
			CHAT_REQ,			// 19
			CHAT,					// 20
			SETUP_GAME,			// 21
			START_GAME,			// 22
			ABORT_GAME,			// 23
			READY_REALM,		// 24
			BAD_REALM,			// 25
			START_REALM,		// 26
			HALT_REALM,			// 27
			NEXT_REALM,			// 28
			PROGRESS_REALM,	// 29
			PROCEED,				// 30
			PING,					// 31
			RAND,					// 32

			// Total number of messages (must be the last value)
			NumMessages
			};

		// Reasons client can't join
		typedef enum
			{
			TooManyPlayers,
			BandwidthTooLow,
			GameAlreadyStarted
			};

		// Reasons why game was aborted
		typedef enum
			{
			UserAbortedGame,
			ErrorAbortedGame
			};

		// Reasons for errors
		typedef enum
			{
			NoError,
			ReceiveError,
			InQFullError,
			OutQFullError,
			SendError,
			InQReadError,
			OutQWriteError,
			ConnectionError,
			TimeoutError,
			ListenError,
			CantConnectError,
			ConnectTimeoutError,
			ServerVersionMismatchError,
			LoginDeniedError,
			JoinDeniedError,
			CantOpenPeerSocketError,
			ClientVersionMismatchError,
			ServerPlatformMismatchError,
			ClientPlatformMismatchError,
			// Keep this as last value
			NumErrors
			} Error;

		// Reasons for status
		typedef enum
			{
			NoStatus,
			Opened,
			Connected,
			LoginAccepted,
			JoinAccepted,
			// Keep this as last value
			NumStatii
			} Status;


		//------------------------------------------------------------------------------
		// Typedefs for each of the message structs.
		//
		// Aside from some no-longer-valid historical reasons why these started out as
		// structs instead of classes, there are still some reasons why they remain
		// structs instead of classes.  With a union of classes, we can declare the
		// "container" struct (NetMsg) and change it into any message we like by simply
		// changing the message type.  With a class, if we declare the base class, we
		// only get enough data for the base class itself.  We could probably declare
		// a union of classes, but that would then include memory for virtual function
		// pointers (really??), and I feel less comfortable casting classes back and
		// forth into other classes than I do with structs.  Not GREAT reasons, but
		// that's why it is what it is.
		//------------------------------------------------------------------------------

		typedef struct Nothing
			{
			enum { Size = 1 };
			unsigned char	ucType;								// Message type

			long lSize;												// Size of message for variable-sized messages
																		// This is not an integral part of the Nothing
																		//	message, but rather it's a way for us to
																		// generically access the size member of a
																		// variable-sized message.  For those messages,
																		// the size always follows the type.

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				TRACE("ERROR!  NOTHING message is not intended to be transmitted!\n");
				ASSERT(0);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				TRACE("ERROR!  NOTHING message is not intended to be transmitted!\n");
				ASSERT(0);
				}
			} Nothing;

		typedef struct Stat
			{
			enum { Size = 1 + sizeof(Status) };
			unsigned char	ucType;								// Message type
			Status			status;								// status value

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				TRACE("ERROR!  STAT message is not intended to be transmitted!\n");
				ASSERT(0);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				TRACE("ERROR!  STAT message is not intended to be transmitted!\n");
				ASSERT(0);
				}
			} Stat;

		typedef struct Err
			{
			enum { Size = 1 + sizeof(Error) + sizeof(ULONG) };
			unsigned char	ucType;								// Message type
			Error				error;								// error value
			ULONG				ulParam;								// Miscellaneous param for errors.
																		// For *VersionMismatchError, used
																		// for other system's version for
																		// error reportage.

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				TRACE("ERROR!  ERR message is not intended to be transmitted!\n");
				ASSERT(0);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				TRACE("ERROR!  ERR message is not intended to be transmitted!\n");
				ASSERT(0);
				}
			} Err;

		// client tells server it wants to login
		typedef struct Login
			{
			enum { Size = 9 };
			unsigned char	ucType;								// Message type
			unsigned long	ulMagic;								// Magic number
			unsigned long	ulVersion;							// Version number

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.login.ucType);
				pBuf->Get(&pmsg->msg.login.ulMagic);
				pBuf->Get(&pmsg->msg.login.ulVersion);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.login.ucType);
				pBuf->Put(&pmsg->msg.login.ulMagic);
				pBuf->Put(&pmsg->msg.login.ulVersion);
				}
			} Login;

		// server tells client it may login
		typedef struct LoginAccept
			{
			enum { Size = 10 };
			unsigned char	ucType;								// Message type
			Net::ID idAssigned;									// Assigned ID
			unsigned long	ulMagic;								// Magic number
			unsigned long	ulVersion;							// Version number

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.loginAccept.ucType);
				pBuf->Get(&pmsg->msg.loginAccept.idAssigned);
				pBuf->Get(&pmsg->msg.loginAccept.ulMagic);
				pBuf->Get(&pmsg->msg.loginAccept.ulVersion);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.loginAccept.ucType);
				pBuf->Put(&pmsg->msg.loginAccept.idAssigned);
				pBuf->Put(&pmsg->msg.loginAccept.ulMagic);
				pBuf->Put(&pmsg->msg.loginAccept.ulVersion);
				}
			} LoginAccept;

		// server tells client it may not login
		typedef struct LoginDeny
			{
			enum { Size = 1 };
			unsigned char	ucType;								// Message type

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.loginDeny.ucType);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.loginDeny.ucType);
				}
			} LoginDeny;

		// client tells server it is logging out
		typedef struct Logout
			{
			enum { Size = 1 };
			unsigned char	ucType;								// Message type

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.logout.ucType);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.logout.ucType);
				}
			} Logout;

		// client tells server it wants to join
		typedef struct JoinReq
			{
			enum { Size = 1 + Net::MaxPlayerNameSize + 1 + 1 + 2 };
			unsigned char	ucType;								// Message type
			char				acName[Net::MaxPlayerNameSize];// Name
			unsigned char	ucColor;								// Color number
			unsigned char	ucTeam;								// Team number
			short				sBandwidth;							// Bandwidth

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.joinReq.ucType);
				pBuf->Get(pmsg->msg.joinReq.acName, sizeof(pmsg->msg.joinReq.acName));
				pBuf->Get(&pmsg->msg.joinReq.ucColor);
				pBuf->Get(&pmsg->msg.joinReq.ucTeam);
				pBuf->Get(&pmsg->msg.joinReq.sBandwidth);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.joinReq.ucType);
				pBuf->Put(pmsg->msg.joinReq.acName, sizeof(pmsg->msg.joinReq.acName));
				pBuf->Put(&pmsg->msg.joinReq.ucColor);
				pBuf->Put(&pmsg->msg.joinReq.ucTeam);
				pBuf->Put(&pmsg->msg.joinReq.sBandwidth);
				}
			} JoinReq;

		// server tells client it can join
		typedef struct JoinAccept
			{
			enum { Size = 1 };
			unsigned char	ucType;								// Message type

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.joinAccept.ucType);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.joinAccept.ucType);
				}
			} JoinAccept;

		// server tells client it cannot join
		typedef struct JoinDeny
			{
			enum { Size = 2 };
			unsigned char	ucType;								// Message type
			unsigned char	ucReason;							// Reason for denail

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.joinDeny.ucType);
				pBuf->Get(&pmsg->msg.joinDeny.ucReason);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.joinDeny.ucType);
				pBuf->Put(&pmsg->msg.joinDeny.ucReason);
				}
			} JoinDeny;

		// server tells client about another client (or about itself!)
		typedef struct Joined
			{
			enum { Size = 1 + 1 + sizeof(RSocket::Address) + Net::MaxPlayerNameSize + 1 + 1 + 2 };

			unsigned char	ucType;								// Message type
			Net::ID id;												// ID
			RSocket::Address address;							// Address
			char				acName[Net::MaxPlayerNameSize];// Name
			unsigned char	ucColor;								// Color number
			unsigned char	ucTeam;								// Team number
			short				sBandwidth;							// Bandwidth

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.joined.ucType);
				pBuf->Get(&pmsg->msg.joined.id);
				GetSocketAddress(pBuf, &pmsg->msg.joined.address);
				pBuf->Get(pmsg->msg.joined.acName, sizeof(pmsg->msg.joined.acName));
				pBuf->Get(&pmsg->msg.joined.ucColor);
				pBuf->Get(&pmsg->msg.joined.ucTeam);
				pBuf->Get(&pmsg->msg.joined.sBandwidth);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.joined.ucType);
				pBuf->Put(&pmsg->msg.joined.id);
				PutSocketAddress(pBuf, &pmsg->msg.joined.address);
				pBuf->Put(pmsg->msg.joined.acName, sizeof(pmsg->msg.joined.acName));
				pBuf->Put(&pmsg->msg.joined.ucColor);
				pBuf->Put(&pmsg->msg.joined.ucTeam);
				pBuf->Put(&pmsg->msg.joined.sBandwidth);
				}
			} Joined;

		// client tells server it wants to change its info
		typedef struct ChangeReq
			{
			enum { Size = 1 + Net::MaxPlayerNameSize + 1 + 1 };
			unsigned char	ucType;								// Message type
			char				acName[Net::MaxPlayerNameSize];// Name
			unsigned char	ucColor;								// Color number
			unsigned char	ucTeam;								// Team number

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.changeReq.ucType);
				pBuf->Get(pmsg->msg.changeReq.acName, sizeof(pmsg->msg.changeReq.acName));
				pBuf->Get(&pmsg->msg.changeReq.ucColor);
				pBuf->Get(&pmsg->msg.changeReq.ucTeam);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.changeReq.ucType);
				pBuf->Put(pmsg->msg.changeReq.acName, sizeof(pmsg->msg.changeReq.acName));
				pBuf->Put(&pmsg->msg.changeReq.ucColor);
				pBuf->Put(&pmsg->msg.changeReq.ucTeam);
				}
			} ChangeReq;

		// server tells client about a client's change
		typedef struct Changed
			{
			enum { Size = 1 + 1 + Net::MaxPlayerNameSize + 1 + 1 };
			unsigned char	ucType;								// Message type
			Net::ID id;												// ID
			char				acName[Net::MaxPlayerNameSize];// Name
			unsigned char	ucColor;								// Color number
			unsigned char	ucTeam;								// Team number

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.changed.ucType);
				pBuf->Get(&pmsg->msg.changed.id);
				pBuf->Get(pmsg->msg.changed.acName, sizeof(pmsg->msg.changed.acName));
				pBuf->Get(&pmsg->msg.changed.ucColor);
				pBuf->Get(&pmsg->msg.changed.ucTeam);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.changed.ucType);
				pBuf->Put(&pmsg->msg.changed.id);
				pBuf->Put(pmsg->msg.changed.acName, sizeof(pmsg->msg.changed.acName));
				pBuf->Put(&pmsg->msg.changed.ucColor);
				pBuf->Put(&pmsg->msg.changed.ucTeam);
				}
			} Changed;

		// client tells server it wants to drop
		typedef struct DropReq
			{
			enum { Size = 1 };
			unsigned char	ucType;								// Message type

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.dropReq.ucType);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.dropReq.ucType);
				}
			} DropReq;

		// Server ---> Client
		//
		// When the server sends one of these before the game has started,
		// sContext will be -1, which indicates that no response is
		// required.  All the clients simply drop that player.
		//
		// When the server sends one of these during a game, sContext
		// will be >= 0.  It is used when multiple players are being dropped
		// at the same time.  All further drop messages related to a particular
		// drop context must contain this context number.  The server and the
		// clients all pause their games throughout a drop context.  Clients
		// must respond to this message with a DropAck.
		typedef struct Dropped
			{
			enum { Size = 1 + 1 + 2 };
			unsigned char	ucType;								// Message type
			Net::ID id;												// Dropee's ID
			short sContext;										// Context or -1 if none

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.dropped.ucType);
				pBuf->Get(&pmsg->msg.dropped.id);
				pBuf->Get(&pmsg->msg.dropped.sContext);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.dropped.ucType);
				pBuf->Put(&pmsg->msg.dropped.id);
				pBuf->Put(&pmsg->msg.dropped.sContext);
				}
			} Dropped;

		// Client ---> Server
		//
		// When clients receive a Dropped message with a valid drop
		// context (not -1), they pause their game and respond with this
		// message, including the drop context within the message so the
		// server knows which context the message belongs to.  This message
		// tells the server what the client's client's last known
		// input sequence from the dropped player is, along with what
		// the client's current frame number is.
		// 
		// The server uses these responses to determine which client knows
		// the most about the dropee and which client is furthest ahead
		// in terms of frames.  From that, it can also determine how much of
		// the dropee's input data to send to each of the clients so that
		// they all have the same information.
		typedef struct DropAck
			{
			enum { Size = 1 + sizeof(Net::SEQ) + sizeof(Net::SEQ) };
			unsigned char	ucType;								// Message type
			Net::SEQ seqLastDropeeInput;						// Client's last input seq from dropee
			Net::SEQ seqLastDoneFrame;							// Client's last frame that was done

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.dropAck.ucType);
				pBuf->Get(&pmsg->msg.dropAck.seqLastDropeeInput);
				pBuf->Get(&pmsg->msg.dropAck.seqLastDoneFrame);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.dropAck.ucType);
				pBuf->Put(&pmsg->msg.dropAck.seqLastDropeeInput);
				pBuf->Put(&pmsg->msg.dropAck.seqLastDoneFrame);
				}
			} DropAck;

		// Server ---> Client
		//
		// The server sends these to a client to request that the client
		// supply the range of input data for the specified net ID.
		//
		// Previous to this message, the server would have learned
		// what range of inputs the client had available by way of
		// the DropAck that the client sent to the server.
		typedef struct InputReq
			{
			enum { Size = 1 + 1 + sizeof(Net::SEQ) + 2 };
			unsigned char	ucType;								// Message type
			Net::ID id;												// ID whose input is being requested
			Net::SEQ seqStart;									// Startng seq of range
			short sNum;												// Number of seq's in range

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.inputReq.ucType);
				pBuf->Get(&pmsg->msg.inputReq.id);
				pBuf->Get(&pmsg->msg.inputReq.seqStart);
				pBuf->Get(&pmsg->msg.inputReq.sNum);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.inputReq.ucType);
				pBuf->Put(&pmsg->msg.inputReq.id);
				pBuf->Put(&pmsg->msg.inputReq.seqStart);
				pBuf->Put(&pmsg->msg.inputReq.sNum);
				}
			} InputReq;

		// Client <---> Server
		// Variable sized message!
		//
		// The client sends this in response to the server's InputReq.
		typedef struct InputData
			{
			enum { Size = -1 };									// -1 indicates variable-sized message
			unsigned char	ucType;								// Message type
			long lSize;												// Message size (must follow type!)
			Net::ID id;												// ID whose input is being sent
			Net::SEQ seqStart;									// Starting seq of range
			short sNum;												// Number of seq's in range
			UINPUT* pInputs;										// Pointer used to read/write actual input data
			U8*		pFrameTimes;									// Pointer to read/write actual frame time data *SPA

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.inputData.ucType);
				pBuf->Get(&pmsg->msg.inputData.lSize);
				pBuf->Get(&pmsg->msg.inputData.id);
				pBuf->Get(&pmsg->msg.inputData.seqStart);
				pBuf->Get(&pmsg->msg.inputData.sNum);
				
				// Allocate buffer for variable-sized data
				pmsg->msg.inputData.pInputs = (UINPUT*)AllocVar(pmsg, (long)pmsg->msg.inputData.sNum * sizeof(UINPUT));

				// Get the variable-sized data
				pBuf->Get(pmsg->msg.inputData.pInputs, (long)(pmsg->msg.inputData.sNum));
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				// Calculate message size
				pmsg->msg.inputData.lSize = 1 + 4 + 1 + sizeof(Net::SEQ) + 2 + ((long)(pmsg->msg.inputData.sNum) * sizeof(UINPUT));

				// Write message
				pBuf->Put(&pmsg->msg.inputData.ucType);
				pBuf->Put(&pmsg->msg.inputData.lSize);
				pBuf->Put(&pmsg->msg.inputData.id);
				pBuf->Put(&pmsg->msg.inputData.seqStart);
				pBuf->Put(&pmsg->msg.inputData.sNum);
				pBuf->Put(pmsg->msg.inputData.pInputs, (long)(pmsg->msg.inputData.sNum));
				}
			} InputData;

		// Server ---> Client
		//
		// The server sends these to a client to tell the client
		// to mark the specified client's input data as being
		// invalid after the specified seq.
		typedef struct InputMark
			{
			enum { Size = 1 + 1 + sizeof(Net::SEQ) };
			unsigned char	ucType;								// Message type
			Net::ID id;												// ID whose input is being requested
			Net::SEQ seqMark;										// Seq to mark

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.inputMark.ucType);
				pBuf->Get(&pmsg->msg.inputMark.id);
				pBuf->Get(&pmsg->msg.inputMark.seqMark);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.inputMark.ucType);
				pBuf->Put(&pmsg->msg.inputMark.id);
				pBuf->Put(&pmsg->msg.inputMark.seqMark);
				}
			} InputMark;

		// client tells server it wants to chat
		typedef struct ChatReq
			{
			enum { Size = 1 + 2 + Net::MaxChatSize };

			unsigned char	ucType;								// Message type
			U16				u16Mask;								// Who will get this chat text
			char				acText[Net::MaxChatSize];		// Chat text

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.chatReq.ucType);
				pBuf->Get(&pmsg->msg.chatReq.u16Mask);
				pBuf->Get(pmsg->msg.chatReq.acText, sizeof(pmsg->msg.chatReq.acText));
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.chatReq.ucType);
				pBuf->Put(&pmsg->msg.chatReq.u16Mask);
				pBuf->Put(pmsg->msg.chatReq.acText, sizeof(pmsg->msg.chatReq.acText));
				}
			} ChatReq;
		
		// server tells specified clients the chat message
		typedef struct Chat
			{
			enum { Size = 1 + 1 + Net::MaxChatSize };

			unsigned char	ucType;								// Message type
			unsigned char	id;									// Who said this
			char				acText[Net::MaxChatSize];		// Chat text

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.chat.ucType);
				pBuf->Get(&pmsg->msg.chat.id);
				pBuf->Get(pmsg->msg.chat.acText, sizeof(pmsg->msg.chat.acText));
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.chat.ucType);
				pBuf->Put(&pmsg->msg.chat.id);
				pBuf->Put(pmsg->msg.chat.acText, sizeof(pmsg->msg.chat.acText));
				}
			} Chat;

		// server tells clients how to setup game
		typedef struct SetupGame
			{
			enum { Size = 1 + 2 + Net::MaxRealmNameSize + 2 + 2 + 2 + 2 + 2 + 2 };

			unsigned char	ucType;								// Message type
			short				sRealmNum;							// Starting realm number or -1 to use name
			char				acRealmFile[Net::MaxRealmNameSize];// Name of realm file to load
			short				sDifficulty;						// Difficulty level
			short				sRejuvenate;
			short				sTimeLimit;
			short				sKillLimit;
			short				sCoopLevels;						// Non-zero for cooperative levels, zero for deathmatch levels.
			short				sCoopMode;							// Non-zero for cooperative mode, zero for deathmatch mode.

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.setupGame.ucType);
				pBuf->Get(&pmsg->msg.setupGame.sRealmNum);
				pBuf->Get(pmsg->msg.setupGame.acRealmFile, sizeof(pmsg->msg.setupGame.acRealmFile));
				pBuf->Get(&pmsg->msg.setupGame.sDifficulty);
				pBuf->Get(&pmsg->msg.setupGame.sRejuvenate);
				pBuf->Get(&pmsg->msg.setupGame.sTimeLimit);
				pBuf->Get(&pmsg->msg.setupGame.sKillLimit);
				pBuf->Get(&pmsg->msg.setupGame.sCoopLevels);
				pBuf->Get(&pmsg->msg.setupGame.sCoopMode);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.setupGame.ucType);
				pBuf->Put(&pmsg->msg.setupGame.sRealmNum);
				pBuf->Put(pmsg->msg.setupGame.acRealmFile, sizeof(pmsg->msg.setupGame.acRealmFile));
				pBuf->Put(&pmsg->msg.setupGame.sDifficulty);
				pBuf->Put(&pmsg->msg.setupGame.sRejuvenate);
				pBuf->Put(&pmsg->msg.setupGame.sTimeLimit);
				pBuf->Put(&pmsg->msg.setupGame.sKillLimit);
				pBuf->Put(&pmsg->msg.setupGame.sCoopLevels);
				pBuf->Put(&pmsg->msg.setupGame.sCoopMode);
				}
			} SetupGame;

		// server tells clients to start game
		typedef struct StartGame
			{
			enum { Size = 1 + 1 + 2 + Net::MaxRealmNameSize + 2 + 2 + 2 + 2 + 2 + sizeof(Net::SEQ) + 2 + 2};
			unsigned char	ucType;												// Message type
			Net::ID			idServer;											// Server's ID
			short				sRealmNum;											// Starting realm number or -1 to use name
			char				acRealmFile[Net::MaxRealmNameSize];			// Name of realm file to load
			short				sDifficulty;										// Difficulty level
			short				sRejuvenate;
			short				sTimeLimit;
			short				sKillLimit;
			short				sCoopLevels;										// Non-zero for cooperative levels, zero for deathmatch levels.
			short				sCoopMode;											// Non-zero for cooperative mode, zero for deathmatch mode.
			short				sFrameTime;											// Milliseconds per frame
			Net::SEQ			seqMaxAhead;										// Max ahead for input versus frame seq

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.startGame.ucType);
				pBuf->Get(&pmsg->msg.startGame.idServer);
				pBuf->Get(&pmsg->msg.startGame.sRealmNum);
				pBuf->Get(pmsg->msg.startGame.acRealmFile, sizeof(pmsg->msg.setupGame.acRealmFile));
				pBuf->Get(&pmsg->msg.startGame.sDifficulty);
				pBuf->Get(&pmsg->msg.startGame.sRejuvenate);
				pBuf->Get(&pmsg->msg.startGame.sTimeLimit);
				pBuf->Get(&pmsg->msg.startGame.sKillLimit);
				pBuf->Get(&pmsg->msg.startGame.sCoopLevels);
				pBuf->Get(&pmsg->msg.startGame.sCoopMode);
				pBuf->Get(&pmsg->msg.startGame.sFrameTime);
				pBuf->Get(&pmsg->msg.startGame.seqMaxAhead);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.startGame.ucType);
				pBuf->Put(&pmsg->msg.startGame.idServer);
				pBuf->Put(&pmsg->msg.startGame.sRealmNum);
				pBuf->Put(pmsg->msg.startGame.acRealmFile, sizeof(pmsg->msg.setupGame.acRealmFile));
				pBuf->Put(&pmsg->msg.startGame.sDifficulty);
				pBuf->Put(&pmsg->msg.startGame.sRejuvenate);
				pBuf->Put(&pmsg->msg.startGame.sTimeLimit);
				pBuf->Put(&pmsg->msg.startGame.sKillLimit);
				pBuf->Put(&pmsg->msg.startGame.sCoopLevels);
				pBuf->Put(&pmsg->msg.startGame.sCoopMode);
				pBuf->Put(&pmsg->msg.startGame.sFrameTime);
				pBuf->Put(&pmsg->msg.startGame.seqMaxAhead);
				}
			} StartGame;

		// server tells clients to abort game
		typedef struct AbortGame
			{
			enum { Size = 1 + 1 };
			unsigned char	ucType;								// Message type
			unsigned char	ucReason;							// Reason for abort

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.abortGame.ucType);
				pBuf->Get(&pmsg->msg.abortGame.ucReason);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.abortGame.ucType);
				pBuf->Put(&pmsg->msg.abortGame.ucReason);
				}
			} AbortGame;

		// client tells server the loaded realm is ready
		typedef struct ReadyRealm
			{
			enum { Size = 1 };
			unsigned char	ucType;								// Message type

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.readyRealm.ucType);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.readyRealm.ucType);
				}
			} ReadyRealm;

		// client tells server the load failed
		typedef struct BadRealm
			{
			enum { Size = 1 };
			unsigned char	ucType;								// Message type

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.badRealm.ucType);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.badRealm.ucType);
				}
			} BadRealm;

		// server tells clients to start realm
		typedef struct StartRealm
			{
			enum { Size = 1 };
			unsigned char	ucType;								// Message type

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.startRealm.ucType);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.startRealm.ucType);
				}
			} StartRealm;

		// server tells clients to start realm
		typedef struct HaltRealm
			{
			enum { Size = 1 + sizeof(Net::SEQ) };
			unsigned char	ucType;								// Message type
			Net::SEQ seqHalt;										// Which seq to halt on

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.haltRealm.ucType);
				pBuf->Get(&pmsg->msg.haltRealm.seqHalt);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.haltRealm.ucType);
				pBuf->Put(&pmsg->msg.haltRealm.seqHalt);
				}
			} HaltRealm;

		// server tells clients to go to next realm when the specified frame seq is reached
		typedef struct NextRealm
			{
			enum { Size = 1 + sizeof(Net::SEQ) };
			unsigned char	ucType;								// Message type
			Net::SEQ seqHalt;										// Which seq to halt on

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.nextRealm.ucType);
				pBuf->Get(&pmsg->msg.haltRealm.seqHalt);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.nextRealm.ucType);
				pBuf->Put(&pmsg->msg.haltRealm.seqHalt);
				}
			} NextRealm;

		// server tells clients to procceed, which is used when clients are at a dialog
		// or something like that which requires a centralized "user input"
		typedef struct ProgressRealm
			{
			enum { Size = 1 + 2 + 2 };
			unsigned char	ucType;								// Message type
			short sNumReady;										// Number of players that are ready
			short sNumNotReady;									// Number of players that are NOT ready

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.progressRealm.ucType);
				pBuf->Get(&pmsg->msg.progressRealm.sNumReady);
				pBuf->Get(&pmsg->msg.progressRealm.sNumNotReady);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.progressRealm.ucType);
				pBuf->Put(&pmsg->msg.progressRealm.sNumReady);
				pBuf->Put(&pmsg->msg.progressRealm.sNumNotReady);
				}
			} ProgressRealm;

		// server tells clients to procceed, which is used when clients are at a dialog
		// or something like that which requires a centralized "user input"
		typedef struct Proceed
			{
			enum { Size = 1 };
			unsigned char	ucType;								// Message type

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.proceed.ucType);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.proceed.ucType);
				}
			} Proceed;

		// ping (back and forth between client and server)
		typedef struct Ping
			{
			enum { Size = 1 + 4 + 4 };
			unsigned char	ucType;								// Message type
			long				lTimeStamp;							// Timestap for this ping
			long				lLatestPingResult;				// Latest ping result (round trip time)

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.ping.ucType);
				pBuf->Get(&pmsg->msg.ping.lTimeStamp);
				pBuf->Get(&pmsg->msg.ping.lLatestPingResult);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.ping.ucType);
				pBuf->Put(&pmsg->msg.ping.lTimeStamp);
				pBuf->Put(&pmsg->msg.ping.lLatestPingResult);
				}
			} Ping;

		// debug message used to compare random number sequences
		typedef struct Rand
			{
			enum { Size = 1 + 4 + 4 };
			unsigned char	ucType;								// Message type
			long				lFrame;								// Current frame
			long				lRand;								// Current rand()

			static void Read(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Get(&pmsg->msg.rand.ucType);
				pBuf->Get(&pmsg->msg.rand.lFrame);
				pBuf->Get(&pmsg->msg.rand.lRand);
				}

			static void Write(NetMsg* pmsg, CBufQ* pBuf)
				{
				pBuf->Put(&pmsg->msg.rand.ucType);
				pBuf->Put(&pmsg->msg.rand.lFrame);
				pBuf->Put(&pmsg->msg.rand.lRand);
				}
			} Rand;


	//------------------------------------------------------------------------------
	// The actual data of the NetMsg is a union of all the different message types,
	// each with (mostly) different members, plus some additional data tacked on
	// the end WHICH IS NOT ACTUALLY TRANSMITTED AS PART OF THE MESSAGES!
	//------------------------------------------------------------------------------

	public:
		union
			{
			Nothing			nothing;
			Stat				stat;
			Err				err;
			Login				login;
			LoginAccept		loginAccept;
			LoginDeny		loginDeny;
			Logout			logout;
			JoinReq			joinReq;
			JoinAccept		joinAccept;
			JoinDeny			joinDeny;
			Joined			joined;
			ChangeReq		changeReq;
			Changed			changed;
			DropReq			dropReq;
			Dropped			dropped;
			DropAck			dropAck;
			InputReq			inputReq;
			InputData		inputData;
			InputMark		inputMark;
			ChatReq			chatReq;
			Chat				chat;
			SetupGame		setupGame;
			StartGame		startGame;
			AbortGame		abortGame;
			ReadyRealm		readyRealm;
			BadRealm			badRealm;
			StartRealm		startRealm;
			HaltRealm		haltRealm;
			NextRealm		nextRealm;
			ProgressRealm	progressRealm;
			Proceed			proceed;
			Ping				ping;
			Rand				rand;
			} msg;

		// This is not sent as part of the message.  It is filled in at the receiving
		// end to indicate the sender of the message.  Note that this is NOT handled
		// by either NetMsg or CNetMsgr, which don't know who sent what.
		unsigned char ucSenderID;

	protected:
		// This is not sent as part of the message.  It is used by variable-length
		// messages that require a separate memory block for their data.  Note that
		// the size refers to the size of this data, not the whole msg.
		U8*	m_pVarData;
		long	m_lVarSize;

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		NetMsg()
			{
			m_pVarData = 0;
			m_lVarSize = 0;
			msg.nothing.ucType = NOTHING;
			}

		~NetMsg()
			{
			FreeVar(this);
			}

		void Reset(void)
			{
			FreeVar(this);
			msg.nothing.ucType = NOTHING;
			}

		U8* AllocVar(
			long lSize)
			{
			FreeVar();
			m_lVarSize = lSize;
			m_pVarData = new U8[lSize];
			return m_pVarData;
			}

		void FreeVar(void)
			{
			delete []m_pVarData;
			m_pVarData = 0;
			m_lVarSize = 0;
			}

		static U8* AllocVar(
			NetMsg* pmsg,
			long lSize)
			{
			return pmsg->AllocVar(lSize);
			}

		static void FreeVar(
			NetMsg* pmsg)
			{
			pmsg->FreeVar();
			}

		const NetMsg& operator=(const NetMsg& rhs)
			{
			// Copy message
			msg = rhs.msg;

			// Copy sender ID
			ucSenderID = rhs.ucSenderID;

			// Copy var memory, if any
			FreeVar(this);
			if (rhs.m_pVarData)
				{
				AllocVar(this, rhs.m_lVarSize);
				memcpy(m_pVarData, rhs.m_pVarData, rhs.m_lVarSize);
				}

			return *this;
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// CNetMsgr impliments a message-based protocol over a "reliable" network
// socket connection.
//
////////////////////////////////////////////////////////////////////////////////
class CNetMsgr
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:
		// Miscellaneous values
		typedef enum
			{
			MagicNum				= 0x5655595a,					// Magic number
			MacVersionBit		= 0x1000,						// Bit that indicates a Mac platform.
			MinVersionNum		= 0x0003,						// Minimum version number we can support
			CurVersionNum		= 0x0003							// Current version number
			};

		// States
		typedef enum
			{
			Disconnected,			// Becomes "Connecting" when Connect() is called

			Connecting,				// Becomes "Disconnected" if connection attempt fails
										// Becomes "Connected" if connection attempt succeeds

			Connected,				// Becomes "Disconnected" if immediate disconnect is performed
										// Becomes "Disconnecting" if clean disconnect is performed

			Disconnecting			// Becomes "Disconnected" when disconnection process finishes
			} State;

		// Info about a message
		typedef void (*FUNC_READ)(NetMsg* pmsg, CBufQ* pBuf);	// Pointer to read function
		typedef void (*FUNC_WRITE)(NetMsg* pmsg, CBufQ* pBuf);	// Pointer to write function
		typedef struct
			{
			UCHAR				ucType;								// Type (used only for debug ASSERTs)
			size_t			size;									// Size of data to read/write (bytes)
			FUNC_READ		funcRead;							// Pointer to read function
			FUNC_WRITE		funcWrite;							// Pointer to write function
			} InfoMsg;

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		State					m_state;								// Current state		
		RSocket				m_socket;							// Socket
		RSocket::Address	m_address;							// Address we're connected to
		CBufQ					m_bufIn;								// Input buffer
		CBufQ					m_bufOut;							// Output buffer
		long					m_lMsgRecvTime;					// When most-recent message was recieved
		long					m_lMsgSentTime;					// When most-recent message was sent
		NetMsg::Error		m_error;								// Error value.

	// Made public by JMB for TAPI access
	public:
		static InfoMsg		ms_aInfoMsg[NetMsg::NumMessages];	// Information about the message structs

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CNetMsgr()
			{
			Reset();
			}

		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CNetMsgr()
			{
			Reset();
			}

		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		void Reset(
			bool bCleanly = false)
			{
			m_state = Disconnected;
			if (bCleanly)
				m_socket.Close(false);	// This tries to close the socket cleanly
			else
				m_socket.Reset();			// This does a more forcefull closing of the socket
			m_address.Reset();
			m_bufIn.Reset();
			m_bufOut.Reset();
			m_lMsgRecvTime = 0;
			m_lMsgSentTime = 0;
			m_error = NetMsg::NoError;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Connect
		//
		// Connecting is an ASYNCHRONOUS operation.  Call this function once to start
		// the connection process, and thereafter call Update() to give it time to
		// process, and call State() to determine the current state.
		//
		// If this function returns failure, it means the attempt to connect failed,
		// and m_state will be "Disconnected".
		//
		// If this function succeeds, it means an attempt to connect has been initiated
		// and m_state will be "Connecting".  The state can be polled to determine the
		// outcome of the connection attempt.  If the state becomes "Connected", it
		// means the connection completed.  If it becomes "Disconnected", it means the
		// connection failed.
		////////////////////////////////////////////////////////////////////////////////
		short Connect(												// Returns 0 if successfull, non-zero otherwise
			const RSocket::Address* paddress,				// In:  Address being connected to
			RSocket::BLOCK_CALLBACK callback)				// In:  Socket callback
			{
			ASSERT(m_state == Disconnected);

			short sResult = 0;

			// Reset to make sure we're starting with a clean slate
			Reset();

			// Save the address
			m_address = *paddress;

			// Open socket in non-blocking mode
			sResult = m_socket.Open(
				0,
				RSocket::typStream,
				RSocket::optDontWaitOnClose | RSocket::optDontCoalesce | RSocket::optDontBlock,
				callback);
			if (sResult == 0)
				{
				m_state = Connecting;
				}
			else
				{
				TRACE("CNetMsgr::Connect(): Couldn't open socket!\n");
				}

			return sResult;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Accept connection
		////////////////////////////////////////////////////////////////////////////////
		short Accept(
			const RSocket* psocketListen,						// In:  Listen socket to accept connection on
			RSocket::BLOCK_CALLBACK callback)				// In:  Socket callback
			{
			ASSERT(m_state == Disconnected);

			short sResult = psocketListen->Accept(&m_socket, &m_address);
			if (sResult == 0)
				{
				m_socket.SetCallback(callback);
				m_state = Connected;
				m_lMsgRecvTime = rspGetMilliseconds();
				m_lMsgSentTime = rspGetMilliseconds();
				}
			return sResult;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Disconnect cleanly or immediately.
		//
		// Cleanly means the actual disconnect will occur only after all existing
		// messages have been sent or a send error occurs, whichever comes first.
		// This may be the case right now, or it may take a while.  All future
		// SendMsg() calls are ignored once this function is called.
		//
		// NOTE: Update() must be called periodically!!!
		////////////////////////////////////////////////////////////////////////////////
		void Disconnect(
			bool bCleanly = false)								// In:  true means cleanly, false means immediately
			{
			// Disconnect is special case -- don't TRACE/ASSERT if not connected
			if (m_state != Disconnected)
				{
				// If doing immediate disconnect or if there's nothing more to send, then
				// disconnect right now.  Otherwise, just change the state.
				if (!bCleanly || !IsMoreToSend())
					{
					// Disconnect by doing a "nice" reset.  This ensures that everything
					// is properly reset.
					Reset(true);
					}
				else
					m_state = Disconnecting;
				}
			}

		////////////////////////////////////////////////////////////////////////////////
		// Update (must be called regularly)
		////////////////////////////////////////////////////////////////////////////////
		void Update(void);

		////////////////////////////////////////////////////////////////////////////////
		// Get message
		////////////////////////////////////////////////////////////////////////////////
		bool GetMsg(
			NetMsg* pmsg);											// Out: Message is returned here

		////////////////////////////////////////////////////////////////////////////////
		// Send message
		////////////////////////////////////////////////////////////////////////////////
		void SendMsg(
			NetMsg* pmsg,											// In:  Message to send
			bool bSendNow = true);								// In:  Whether to send now or wait until Update()

		////////////////////////////////////////////////////////////////////////////////
		// Determine if there is more data waiting to be sent.  If there is data to
		// to be sent AND there is a send error, then that data can't be sent, so we
		// return false to indicate "no more data".
		////////////////////////////////////////////////////////////////////////////////
		bool IsMoreToSend()										// Returns true if more to send, false otherwise
			{
			return !m_bufOut.IsEmpty() && !IsSendError(m_error);
			}

		////////////////////////////////////////////////////////////////////////////////
		// Determine the current state
		////////////////////////////////////////////////////////////////////////////////
		State GetState()											// Returns current state
			{
			return m_state;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get address we're connected to (address is valid only if we are connected!)
		////////////////////////////////////////////////////////////////////////////////
		const RSocket::Address& GetAddress(void)
			{
			return m_address;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Determine the time of the most-recently received or sent message
		////////////////////////////////////////////////////////////////////////////////
		long GetMostRecentMsgReceiveTime(void)
			{
			return m_lMsgRecvTime;
			}

		long GetMostRecentMsgSentTime(void)
			{
			return m_lMsgSentTime;
			}

	private:
		////////////////////////////////////////////////////////////////////////////////
		// Receive data (copy data from socket into input buffer)
		////////////////////////////////////////////////////////////////////////////////
		void ReceiveData(void);


		////////////////////////////////////////////////////////////////////////////////
		// Send data (copy data output buffer to socket)
		////////////////////////////////////////////////////////////////////////////////
		void SendData(void);


		////////////////////////////////////////////////////////////////////////////////
		// Determine if this is a send-related error
		////////////////////////////////////////////////////////////////////////////////
		bool IsSendError(NetMsg::Error error)
			{
			bool bResult = false;
			switch (error)
				{
				case NetMsg::OutQFullError:
				case NetMsg::SendError:
				case NetMsg::InQReadError:
				case NetMsg::OutQWriteError:
					bResult = true;
					break;
				default:
					break;
				}
			return bResult;
			}
	};


#endif //NETMSGR_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
