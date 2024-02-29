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
// NetClient.cpp
// Project: RSPiX
//
// History:
//		09/01/97 MJR	Nearing the end of a major overhaul.
//
//		09/07/97 MJR	Fixed problem with DROPPED message, whereby the caller
//							couldn't recognize when he himself was dropped because
//							the ID was no longer valid.
//
//							Added support for PROCEED and PROGRESS_REALM messages.
//
//		09/10/97 MJR	Changed return value from StartJoinProcess() so that
//							caller could determine whether the failure was due to
//							unsupported protocol (RSocket::errNotSupported).
//
//		09/12/97 MJR	Added SendText() as alternative to SendChat().
//
//		09/12/97 MJR	Now checks to make sure we're joined before it tries to
//							send chat or text.
//
//		11/25/97	JMI	Changed m_error to m_msgError so we could store a whole
//							error message instead of just the error type.
//							Also, now notices the difference between a version mismatch
//							and a platform mismatch and reports separate errors for
//							each.
//
//		11/26/97	JMI	Masking error in evaluation of version mismatch problem
//							such that platform mismatch was never detected.
//
//		12/18/97	SPA	Changed ReceiveFromPeers to limit by number of times through
//							the loop (to number of joined peers *2) instead of a time limit.
//						Changed SendToPeers to write it's own data directly instead of
//							sending it out over the net just to receive it again later.
//							Also limited the resending of the same packet multiple times
//							to a maximum number (set from prefs - NumSendsPerBurst - default 2)
//							and a repeat interval for burst (set from prefs - SendInterval -
//							default 1000 ms).
//						Added variable frame rate (really more of a self regulating frame rate).
//							Each peer keeps track of how long it took between frames (as measured
//							in CanDoFrame) and then sends that value to all other peers in the 
//							next available frame that hasn't been yet. Each peer then averages the
//							frame times for that frame (which is really the time it took for the 
//							frame that happened MaxFrameLag frames ago). This is stored in an array
//							which contains the average frame times for the last eight frames. The
//							values in this array are then averaged and the result is used as the
//							elapsed game time for the current frame (passed back in psFrameTime).
//							This last step is done to smooth out any rapid changes in the speed.
//						Also changed msg INPUT_REQ and INPUT_DATA to update frame time from above.
//
//		12/22/97 SPA	Rewrote SendToPeers to send only one packet per frame with each packet
//							containing all the frames that that peer might need. This is frames
//							starting from the previous frame (which is the one that we have all
//							the data for) minus MaxFrameLag up to current frame plus MaxFrameLag.
//							This means that we send (MaxFrameLag *2 + 2) frames per packet. This also
//							means that each frame is sent (MaxFrameLag *2 + 2) times which should
//							replace most lost or misplaced packets. In case we do loose multiple
//							packets, there is also a request mechanism. If we are unable to render
//							a frame after a time out has expired, we request frame data from the
//							peers that are missing (as well as resending the last packet sent to 
//							them, just in case the reason he didn't send the data is because he
//							didn't get our data). The request is checked for in ReceiveFromPeers
//							and the requested data sent immediately. To simplify the request mechanism
//							I pulled the center loop of SendToPeers out into it's own method (SendToPeer).
//
//		1/5/98		SPA	Fixed problem were we would not go to next level because the data for the
//							halt frame was not being sent so that everyone was stuck waiting to
//							render the last frame.
//
//		1/6/98	 SPA	TimePerFrame from .ini now used for maximum time per frame instead of actual
//							time per frame. SendInputInterval from .ini now used as timeout value for
//							frame request mechanism.
//
//		06/02/98	JMI	Added an additional condition to avoid indexing into the
//							peers array with our ID when it is invalid in 
//							SetLocalInput().
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "netclient.h"
#include "NetDlg.h"

// This is the maximum size of the peer messages.
#define PEER_MSG_HEADER_SIZE		(1 + 2 + 2 + 2 + 2)
#define PEER_MSG_MAX_SIZE		  (PEER_MSG_HEADER_SIZE + ((Net::MaxAheadSeq * 2) * (sizeof(UINPUT) + sizeof(U8)))) // *SPA


////////////////////////////////////////////////////////////////////////////////
// Get values from buffer in proper endian order (buffer contents are always
// in network order, which is big-endian).
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Get a U8
//------------------------------------------------------------------------------
inline void Get(U8* &pget, U8* pval)
	{
	*pval = *pget++;
	}

//------------------------------------------------------------------------------
// Get an S8
//------------------------------------------------------------------------------
inline void Get(U8* &pget, S8* pval)
	{
	Get(pget, (U8*)pval);
	}

//------------------------------------------------------------------------------
// Get a U16
//------------------------------------------------------------------------------
inline void Get(U8* &pget, U16* pval)
	{
	#ifdef SYS_ENDIAN_LITTLE
		U8* ptmp = ((U8*)(pval)) + 1;
		*ptmp-- = *pget++;
		*ptmp   = *pget++;
	#else
		U8* ptmp = (U8*)(pval);
		*ptmp++ = *pget++;
		*ptmp   = *pget++;
	#endif
	}

//------------------------------------------------------------------------------
// Get an S16
//------------------------------------------------------------------------------
inline void Get(U8* &pget, S16* pval)
	{
	Get(pget, (U16*)pval);
	}

//------------------------------------------------------------------------------
// Get a U32
//------------------------------------------------------------------------------
inline void Get(U8* &pget, U32* pval)
	{
	#ifdef SYS_ENDIAN_LITTLE
		U8* ptmp = ((U8*)(pval)) + 3;
		*ptmp-- = *pget++;
		*ptmp-- = *pget++;
		*ptmp-- = *pget++;
		*ptmp   = *pget++;
	#else
		U8* ptmp = (U8*)(pval);
		*ptmp++ = *pget++;
		*ptmp++ = *pget++;
		*ptmp++ = *pget++;
		*ptmp   = *pget++;
	#endif
	}

//------------------------------------------------------------------------------
// Get a U64
//------------------------------------------------------------------------------
inline void Get(U8* &pget, U64* pval)
{
#ifdef SYS_ENDIAN_LITTLE
	U8* ptmp = ((U8*)(pval)) + 3;
	*ptmp-- = *pget++;
	*ptmp-- = *pget++;
	*ptmp-- = *pget++;
	*ptmp-- = *pget++;
	*ptmp-- = *pget++;
	*ptmp-- = *pget++;
	*ptmp-- = *pget++;
	*ptmp = *pget++;
#else
	U8* ptmp = (U8*)(pval);
	*ptmp++ = *pget++;
	*ptmp++ = *pget++;
	*ptmp++ = *pget++;
	*ptmp++ = *pget++;
	*ptmp++ = *pget++;
	*ptmp++ = *pget++;
	*ptmp++ = *pget++;
	*ptmp = *pget++;
#endif
}

//------------------------------------------------------------------------------
// Get an S32
//------------------------------------------------------------------------------
inline void Get(U8* &pget, S32* pval)
	{
	Get(pget, (U32*)pval);
	}


////////////////////////////////////////////////////////////////////////////////
// Put values into buffer in proper endian order (buffer contents are always
// in network order, which is big-endian).
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Put a U8
//------------------------------------------------------------------------------
inline void Put(U8* &pput, U8 val)
	{
	*pput++ = val;
	}

//------------------------------------------------------------------------------
// Put an S8
//------------------------------------------------------------------------------
inline void Put(U8* &pput, S8 val)
	{
	Put(pput, (U8)val);
	}

//------------------------------------------------------------------------------
// Put a U16
//------------------------------------------------------------------------------
inline void Put(U8* &pput, U16 val)
	{
	#ifdef SYS_ENDIAN_LITTLE
		U8* ptmp = ((U8*)(&val)) + 1;
		*pput++ = *ptmp--;
		*pput++ = *ptmp;
	#else
		U8* ptmp = (U8*)(&val);
		*pput++ = *ptmp++;
		*pput++ = *ptmp;
	#endif
	}

//------------------------------------------------------------------------------
// Put an S16
//------------------------------------------------------------------------------
inline void Put(U8* &pput, S16 val)
	{
	Put(pput, (U16)val);
	}

//------------------------------------------------------------------------------
// Put a U32
//------------------------------------------------------------------------------
inline void Put(U8* &pput, U32 val)
	{
	#ifdef SYS_ENDIAN_LITTLE
		U8*ptmp = ((U8*)(&val)) + 3;
		*pput++ = *ptmp--;
		*pput++ = *ptmp--;
		*pput++ = *ptmp--;
		*pput++ = *ptmp;
	#else
		U8*ptmp = (U8*)(&val);
		*pput++ = *ptmp++;
		*pput++ = *ptmp++;
		*pput++ = *ptmp++;
		*pput++ = *ptmp;
	#endif
	}

//------------------------------------------------------------------------------
// Put a U64
//------------------------------------------------------------------------------
inline void Put(U8* &pput, U64 val)
{
#ifdef SYS_ENDIAN_LITTLE
	U8*ptmp = ((U8*)(&val)) + 3;
	*pput++ = *ptmp--;
	*pput++ = *ptmp--;
	*pput++ = *ptmp--;
	*pput++ = *ptmp--;
	*pput++ = *ptmp--;
	*pput++ = *ptmp--;
	*pput++ = *ptmp--;
	*pput++ = *ptmp;
#else
	U8*ptmp = (U8*)(&val);
	*pput++ = *ptmp++;
	*pput++ = *ptmp++;
	*pput++ = *ptmp++;
	*pput++ = *ptmp++;
	*pput++ = *ptmp++;
	*pput++ = *ptmp++;
	*pput++ = *ptmp++;
	*pput++ = *ptmp;
#endif
}

//------------------------------------------------------------------------------
// Put an S32
//------------------------------------------------------------------------------
inline void Put(U8* &pput, S32 val)
	{
	Put(pput, (U32)val);
	}


////////////////////////////////////////////////////////////////////////////////
// Startup
////////////////////////////////////////////////////////////////////////////////
void CNetClient::Startup(
	RSocket::BLOCK_CALLBACK callback)				// In:  Blocking callback
	{
	// Do a reset to be sure we're starting at a good point
	Reset();

	// Save callback
	m_callback = callback;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown
////////////////////////////////////////////////////////////////////////////////
void CNetClient::Shutdown(void)
	{
	Reset();
	}


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
short CNetClient::StartJoinProcess(					// Returns 0 if successfull, non-zero otherwise
	RSocket::Address* paddressHost,					// In:  Host's address
	char* pszName,											// In:  Joiner's name
	unsigned char ucColor,								// In:  Joiner's color
	unsigned char ucTeam,								// In:  Joiner's team
	short sBandwidth)										// In:  Joiner's Net::Bandwidth
	{
	// Save server address (containing the base port)
	m_addressServer = *paddressHost;

	// Calculate server's listen address (the port is simply offset from the base port)
	m_addressServerListen = *paddressHost;
	RSocket::SetAddressPort(RSocket::GetAddressPort(&m_addressServer) + Net::ListenPortOffset, &m_addressServerListen);

	// Temporarily stash the join data in peer #0
	memcpy(m_aPeers[0].m_acName, pszName, sizeof(m_aPeers[0].m_acName));
	m_aPeers[0].m_acName[sizeof(m_aPeers[0].m_acName)-1] = 0;
	m_aPeers[0].m_ucColor = ucColor;
	m_aPeers[0].m_ucTeam = ucTeam;
	m_aPeers[0].m_sBandwidth = sBandwidth;

	// Start the asynchronous connection process.  If an error is returned, it's
	// never going to succeed.  If it doesn't return an error, we poll m_msgr's
	// state in Update() to determine whether it failed or succeeded.  We also
	// set a timer so that if it takes too long, we'll give up.
	// NOTE: Even though the caller could obviously check the result and not
	// bother calling Update() if the result indicates failure, we do allow
	// for the caller to ignore the value and go right into the normal "loop"
	// which includes calling Update() and GetMsg(), so they can handle errors
	// the "normal" way -- via GetMsg().
	short sResult = m_msgr.Connect(&m_addressServerListen, m_callback);
	m_state = WaitForConnect;
	m_lTimeOut = rspGetMilliseconds() + Net::MaxConnectWaitTime;
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Update (must be called regularly)
////////////////////////////////////////////////////////////////////////////////
void CNetClient::Update(void)
	{
	NetMsg msg;

	// If there's an error message that the user hasn't retrieved yet, then
	// we can't do anything.
	if (m_msgError.msg.err.error == NetMsg::NoError)
		{
		switch (m_state)
			{
			case Nothing:
				// Nothing to do
				break;

			case WaitForConnect:
				// Wait for connect attempt to complete or fail or timeout
				m_msgr.Update();
				switch(m_msgr.GetState())
					{
					case CNetMsgr::Connecting:
						if (rspGetMilliseconds() > m_lTimeOut)
							{
							TRACE("CNetClient::Update(): Timed-out trying to connect to server!\n");
							m_msgError.msg.err.error = NetMsg::ConnectTimeoutError;
							Drop();
							}
						break;

					case CNetMsgr::Connected:
						// Send login message to server and set state to wait for response
						msg.msg.login.ucType = NetMsg::LOGIN;
						msg.msg.login.ulMagic = CNetMsgr::MagicNum;
						msg.msg.login.ulVersion = CNetMsgr::CurVersionNum;
						SendMsg(&msg);
						m_state = WaitForLoginResponse;
						m_status = NetMsg::Connected;
						break;

					case CNetMsgr::Disconnecting:
					case CNetMsgr::Disconnected:
					default:
						TRACE("CNetClient::Update(): Error trying to connect to server!\n");
						m_msgError.msg.err.error = NetMsg::CantConnectError;
						Drop();
						break;
					}
				break;

			case WaitForLoginResponse:
				// Wait for response to our LOGIN message
				m_msgr.Update();
				m_msgr.GetMsg(&msg);
				msg.ucSenderID = Net::InvalidID;
				switch(msg.msg.nothing.ucType)
					{
					case NetMsg::NOTHING:
						break;

					case NetMsg::LOGIN_ACCEPT:
						////////////////////////////////////////////////////////////////
						// Version negotiation and useful error results ////////////////
						////////////////////////////////////////////////////////////////
						//
						// In net implementation for Postal net version 1, both the 
						// server and the client would check the version numbers -- the 
						// server would check msg.login.ulVersion against its 
						// MinVersionNum & CurVersionNum and, if the server accepted the
						// login, the client would check msg.loginAccept.ulVersion 
						// against its MinVersionNum & CurVersionNum.  Although the 
						// version 1 client would display an error message when it 
						// decided it could not support the server's version, the server
						// would refuse a login before that, never giving the client a 
						// chance to notice and report this to the user.
						// To remedy this, the Postal Net version 2's server NEVER 
						// rejects a client and, instaed, relies on the client to reject
						// it.  This gives the client a chance to report the error.
						// Since under our new scheme we never send a LOGIN_DENY, we 
						// know, if we receive a LOGIN_DENY, that we are dealing with a
						// version 1 server.
						//
						// Additionally, bit fifteen of the version number is set if on
						// a Mac platform so we can use the same mechanism to determine
						// if we are attempting to connect a Mac and a PC.
						//
						// Here's the actual scenarios and results for version conflicts:
						//
						// Server 1 vs Client 2 -- Server sends Client a LOGIN_DENY so
						// the client knows it's dealing with a version 1 server and 
						// reports the problem.
						// 
						// Server 2 vs Client 1 -- Server accepts connection.  Client
						// drops, though, when it sees the server's version is 2 and
						// reports the problem.
						//
						// For future versions:
						//
						// Server 3 vs Client 1 -- Server accepts connection.  Client
						// drops, though, when it sees the server's version is 3 and
						// reports the problem.
						// 
						// Server 3 vs Client 2 -- Server accepts connection.  Client
						// drops, though, when it sees the server's version is 3 and
						// reports the problem.
						//
						// (see NetMsg::LOGIN_DENY case for handling of net version 1)
						//
						////////////////////////////////////////////////////////////////

						// Check server's version number to see if we can support it
						if ( (msg.msg.loginAccept.ulVersion >= CNetMsgr::MinVersionNum) && (msg.msg.loginAccept.ulVersion <= CNetMsgr::CurVersionNum))
							{
							// Save our assigned ID
							m_id = msg.msg.loginAccept.idAssigned;

							// Send join request and set state to wait for response.  After this point, we no
							// longer care about the data we temporarily stashed in peer #0 because if our join
							// request is accepted, the server will send us info about ALL the clients, including
							// ourself.  If our request is denied, then nothing matters.
							msg.msg.joinReq.ucType	= NetMsg::JOIN_REQ;
							memcpy(msg.msg.joinReq.acName, m_aPeers[0].m_acName, sizeof(msg.msg.joinReq.acName));
							msg.msg.joinReq.ucColor	= m_aPeers[0].m_ucColor;
							msg.msg.joinReq.ucTeam	= m_aPeers[0].m_ucTeam;
							msg.msg.joinReq.sBandwidth = m_aPeers[0].m_sBandwidth;
							SendMsg(&msg);
							m_state = WaitForJoinResponse;
							m_status = NetMsg::LoginAccepted;
							}
						else
							{
							// Determine error type (possibilities are incompatible 
							// versions and/or incompatible platforms).
							if ( (msg.msg.loginAccept.ulVersion & CNetMsgr::MacVersionBit) ^ (CNetMsgr::MinVersionNum & CNetMsgr::MacVersionBit) )
								{
								// One of us is a Mac and one is a PC.
								m_msgError.msg.err.error = NetMsg::ClientPlatformMismatchError;
								}
							else
								{
								// Incompatible version number.
								m_msgError.msg.err.error	= NetMsg::ClientVersionMismatchError;
								m_msgError.msg.err.ulParam	= msg.msg.loginAccept.ulVersion & ~CNetMsgr::MacVersionBit;
								}

							// Unsupported version number -- send LOGOUT message
							msg.msg.logout.ucType = NetMsg::LOGOUT;
							SendMsg(&msg);
							Drop();
							
							TRACE("CNetClient::Update(): Error trying to login to server -- unsupported version number!\n");
							}
						break;

					case NetMsg::LOGIN_DENY:
						Drop();
						// There's ONLY ONE reason we'll ever get a LOGIN_DENY message and that is
						// a version 1 server refused our connection b/c of our later version number
						// so ... Incompatible version number.
						m_msgError.msg.err.error	= NetMsg::ClientVersionMismatchError;
						m_msgError.msg.err.ulParam	= 1;
//						m_msgError.msg.err.error = NetMsg::LoginDeniedError;
						TRACE("CNetClient::Update(): Login denied!\n");
						break;

					default:
						TRACE("CNetClient::Update(): Unexpected message received while waiting for login response!\n");
						break;
					}
				break;

			case WaitForJoinResponse:
				// Wait for response to our JOIN_REQ message
				m_msgr.Update();
				m_msgr.GetMsg(&msg);
				msg.ucSenderID = Net::InvalidID;
				switch(msg.msg.nothing.ucType)
					{
					case NetMsg::NOTHING:
						break;

					case NetMsg::JOIN_ACCEPT:
						{
						// Calculate our peer port based on the server's base port
						unsigned short usPeerPort = RSocket::GetAddressPort(&m_addressServer) + Net::FirstPeerPortOffset + m_id;

						// Open peer socket (this is an unconnected datagram socket, so all the peers
						// simply hurl their data at this port, and we figure out who it came from).
						if (m_socketPeers.Open(usPeerPort, RSocket::typDatagram, RSocket::optDontBlock, m_callback) == 0)
							{
							// We are now fully joined.  HOWEVER, we don't adjust the m_sNumJoined here
							// because we want to allow the server to be in direct control of that by
							// virtue of JOINED and DROPPED messages.
							m_state = Joined;
							m_status = NetMsg::JoinAccepted;
							}
						else
							{
							Drop();
							m_msgError.msg.err.error = NetMsg::CantOpenPeerSocketError;
							TRACE("CNetClient::Update(): Error opening port!\n");
							}
						}
						break;
					
					case NetMsg::JOIN_DENY:
						Drop();
						m_msgError.msg.err.error = NetMsg::JoinDeniedError;
						TRACE("CNetClient::Update(): Join denied!\n");
						break;
					
					default:
						TRACE("CNetClient::Update(): Unexpected message received while waiting for join response!\n");
						break;
					}
				break;

			case Joined:
				// Update messenger to server
				m_msgr.Update();

				// Always try receiving from peers before sending, because the data we
				// receive can influence the data we'll send.
				ReceiveFromPeers();
				SendToPeers();

				// If nothing was sent in a while, do a ping to tell server we're still alive
				#if NET_PING
					if ((rspGetMilliseconds() - m_msgr.GetMostRecentMsgSentTime()) > MaxTimeBetweenSends)
						{
						// Send ping msg
						msg.msg.ping.ucType = NetMsg::PING;
						msg.msg.ping.lTimeStamp = rspGetMilliseconds();
						msg.msg.ping.lLatestPingResult = m_lLatestPingResult;
						SendMsg(&msg);
						}
				#endif
				break;

			default:
				TRACE("CNetClient::Update(): Unknown state!\n");
				break;
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Get next available message from server
////////////////////////////////////////////////////////////////////////////////
void CNetClient::GetMsg(
	NetMsg* pmsg)											// Out: Message is returned here
	{
	// This indicates whether we got a message to be returned to the caller
	bool bGotMsgForCaller = false;

	if (m_msgError.msg.err.error != NetMsg::NoError)
		{
		// If an error occurred, generate an error message, then reset the error flag
		*pmsg = m_msgError;
		bGotMsgForCaller = true;
		m_msgError.msg.err.error	= NetMsg::NoError;
		m_msgError.msg.err.ulParam	= 0;

		// Return this message to caller
		bGotMsgForCaller = true;
		}
	else if (m_status != NetMsg::NoStatus)
		{
		// If a status occurred, generate a status message, then reset the status flag
		pmsg->msg.stat.ucType = NetMsg::STAT;
		pmsg->msg.stat.status = m_status;
		bGotMsgForCaller = true;
		m_status = NetMsg::NoStatus;

		// Return this message to caller
		bGotMsgForCaller = true;
		}
	else if (m_state == Joined)
		{
		// If we're waiting for the end the of the realm and we've reached the halt
		// frame, then we generate a NEXT_REALM message for the caller.
		if (m_bNextRealmPending && m_bReachedHaltFrame)
			{
			// Clear the flags
			m_bUseHaltFrame = false;
			m_bNextRealmPending = false;
			m_bReachedHaltFrame = false;

			// Create NEXT_REALM message (note that seqHalt is meaningless to caller)
			pmsg->msg.nextRealm.ucType = NetMsg::NEXT_REALM;
			pmsg->msg.nextRealm.seqHalt = 0;

			// Return this message to caller
			bGotMsgForCaller = true;
			}
		else
			{
			// If we're in the joined state, get the next message from the server and
			// fill in the sender ID (it's always the server, which is Net::InvalidID)
			m_msgr.GetMsg(pmsg);
			pmsg->ucSenderID = Net::InvalidID;
			switch (pmsg->msg.nothing.ucType)
				{
				case NetMsg::NOTHING:
					break;

				case NetMsg::STAT:
					// Return this message to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::ERR:
					// If an error occurs, there's no way to recover, so we have to drop.
					// The drop may not fully work if there's a problem communicating with
					// the server, but there's no harm in trying.
					TRACE("CNetClient::GetMsg(): Error message received from messenger!\n");
					Drop();

					// Return this message to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::JOINED:
					{
					// Get id and verify that it's unused
					Net::ID id = pmsg->msg.joined.id;
					ASSERT(m_aPeers[id].m_state == CPeer::Unused);

					// Save this peer's info
					m_aPeers[id].m_address = pmsg->msg.joined.address;
					memcpy(m_aPeers[id].m_acName, pmsg->msg.joined.acName, sizeof(m_aPeers[id].m_acName));
					m_aPeers[id].m_ucColor = pmsg->msg.joined.ucColor;
					m_aPeers[id].m_ucTeam = pmsg->msg.joined.ucTeam;
					
					// Change state to "joined"
					m_aPeers[id].m_state = CPeer::Joined;

					// Adjust number of joined peers.  Note that we rely fully on the server to send the
					// appropriate JOINED and DROPPED messages -- if it screws up, our number will be off.
					m_sNumJoined++;

					// Return this message to caller
					bGotMsgForCaller = true;
					}
					break;

				case NetMsg::CHANGED:
					{
					// Get id and verify that it's joined
					Net::ID id = pmsg->msg.changed.id;
					ASSERT(m_aPeers[id].m_state == CPeer::Joined);

					// Change this peer's info
					memcpy(m_aPeers[id].m_acName, pmsg->msg.changed.acName, sizeof(m_aPeers[id].m_acName));
					m_aPeers[id].m_ucColor = pmsg->msg.changed.ucColor;
					m_aPeers[id].m_ucTeam = pmsg->msg.changed.ucTeam;

					// Return this message to caller
					bGotMsgForCaller = true;
					}
					break;

				case NetMsg::DROPPED:
					{
					// Get id and verify that it's joined
					Net::ID id = pmsg->msg.dropped.id;
					ASSERT(m_aPeers[id].m_state == CPeer::Joined);

					// If it was me that was dropped, it's a different case
					if (id == m_id)
						{
						// We're done playing
						m_bPlaying = false;

						// Disconnect cleanly
						m_msgr.Disconnect(true);

						// Clear net ID
						m_id = Net::InvalidID;

						// Reset state
						m_state = Nothing;

						// Since we cleared our own ID, the caller will no longer be able to
						// recognize himself, because the ID in the message will not match
						// our own ID.  Instead, we change the ID in the message to Net::InvalidID
						// as a flag that indicates "you yourself have been dropped".
						pmsg->msg.dropped.id = Net::InvalidID;
						}
					else
						{
						// If the game has started, dropping is easy.  Otherwise, we merely
						// kick off the beginning of a long sequence...
						if (pmsg->msg.dropped.sContext == -1)
							{
							// Change specified peer's state to "unused"
							m_aPeers[id].m_state = CPeer::Unused;
							}
						else
							{
							// Change specified peer's state to "dropped"
							m_aPeers[id].m_state = CPeer::Dropped;

							// Stop playing until we get a START_REALM, which the server
							// will send when the drop process is completely done.
							m_bPlaying = false;

							// Respond with a DROP_ACK message that tells the server what frame
							// we're on and the last input seq we got from the dropee.
							NetMsg msg;
							msg.msg.dropAck.ucType = NetMsg::DROP_ACK;
							msg.msg.dropAck.seqLastDropeeInput = (Net::SEQ)(m_aPeers[id].m_netinput.FindFirstInvalid() - (Net::SEQ)1);
							msg.msg.dropAck.seqLastDoneFrame = (Net::SEQ)(m_seqFrame - (Net::SEQ)1);
							SendMsg(&msg);
							}
						}

					// Adjust number of joined peers.  Note that we rely fully on the server to send
					// appropriate JOINED and DROPPED messages -- if it screws up, our number will be off.
					m_sNumJoined--;

					// Return this message to caller
					bGotMsgForCaller = true;
					}
					break;

				case NetMsg::INPUT_REQ:
					{
					// Get id
					Net::ID id = pmsg->msg.inputReq.id;

					// In response to this, we need to generate an INPUT_DATA message, which is a
					// variable-size message.  We'll have the message allocate memory for the
					// input data, which is what varies in size.  It will free the memory when it
					// gets destroyed.
					NetMsg msg;
					msg.msg.inputData.ucType	= NetMsg::INPUT_DATA;
					msg.msg.inputData.id			= id;
					msg.msg.inputData.seqStart	= pmsg->msg.inputReq.seqStart;
					msg.msg.inputData.sNum		= pmsg->msg.inputReq.sNum;
					msg.msg.inputData.pInputs	= (UINPUT*)msg.AllocVar((long)pmsg->msg.inputReq.sNum * sizeof(UINPUT));
					msg.msg.inputData.pFrameTimes	= (U8*)msg.AllocVar((long)pmsg->msg.inputReq.sNum * sizeof(U8));
					
					// Copy the requested values into the allocated memory
					Net::SEQ seq = msg.msg.inputData.seqStart;
					for (short s = 0; s < msg.msg.inputData.sNum; s++)
						{
						msg.msg.inputData.pFrameTimes[s] = m_aPeers[id].m_netinput.GetFrameTime(seq); //  *SPA
						msg.msg.inputData.pInputs[s] = m_aPeers[id].m_netinput.Get(seq++);
						}

					// Send to server.  We force this to send immediately with no possibility
					// of buffering because this message will be destroyed upon leaving this
					// section of code, and with it will go the buffer containing all the 
					// inputs.  Obviously, it must be sent BEFORE that happens.
					SendMsg(&msg, true);
					}
					break;

				case NetMsg::INPUT_DATA:
					{
					// Get id and verify that it's for a dropped client, which is the only
					// situation in which we currently use this mechanism.
					Net::ID id = pmsg->msg.inputData.id;
					ASSERT(m_aPeers[id].m_state == CPeer::Dropped);

					// Copy the supplied data to the input buffer for the specified peer.  We
					// have blind faith in the server, and assume it would never send us anything
					// that was bad for us. :)
					Net::SEQ seq = pmsg->msg.inputData.seqStart;
					for (short s = 0; s < pmsg->msg.inputData.sNum; s++)
						{
						m_aPeers[id].m_netinput.PutFrameTime(seq, pmsg->msg.inputData.pFrameTimes[s]); // *SPA
						m_aPeers[id].m_netinput.Put(seq++, pmsg->msg.inputData.pInputs[s]);
						}
					}
					break;

				case NetMsg::INPUT_MARK:
					{
					// Get id and verify that it's for a dropped client, which is the only
					// situation in which we currently use this mechanism.
					Net::ID id = pmsg->msg.inputMark.id;
					ASSERT(m_aPeers[id].m_state == CPeer::Dropped);

					// The specified seq indicates the last seq for which the peer's input buffer
					// should be used.  This value could be ahead of m_seqFrame if other peer's
					// are further ahead than us, or could be equal to m_SeqFrame (which, remember
					// is the frame we're trying to do).  Or, it could be one LESS than m_seqFrame!
					// Consider that if we happen to be the furthest ahead of anyone, then the
					// frame we last did would be the frame the entire drop sequence was based
					// on, so that would be the last valid frame for the dropped peer.  And the
					// last frame we did is one LESS than m_seqFrame.  So, we verify that the
					// specified seq is >= m_seqFrame-1.
					ASSERT(SEQ_GTE(pmsg->msg.inputMark.seqMark, (Net::SEQ)(m_seqFrame - (Net::SEQ)1)));

					// Set peers last active frame
					m_aPeers[id].m_seqLastActive = pmsg->msg.inputMark.seqMark;

					// If the specified seq is m_seqFrame-1, then the peer is already inactive
					// since we already used that input.  Otherwise, it will remain active until
					// we use that input.
					if (m_aPeers[id].m_seqLastActive == (Net::SEQ)(m_seqFrame - (Net::SEQ)1))
						m_aPeers[id].m_bInactive = true;
					else
						m_aPeers[id].m_bInactive = false;
					}
					break;

				case NetMsg::CHAT:
					// Return this message to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::SETUP_GAME:
					// Return this message to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::START_GAME:
					// Get some items of interest out of the message
					m_idServer = pmsg->msg.startGame.idServer;
					m_lFrameTime = (long)pmsg->msg.startGame.sFrameTime;
					m_seqMaxAhead = pmsg->msg.startGame.seqMaxAhead;
					m_seqInputNotYetSent = m_seqMaxAhead + 1; // *SPA

					// Game started, but we always start out "not playing"
					m_bGameStarted = true;
					m_bPlaying = false;

					// Return this message to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::ABORT_GAME:
					// Return this message to caller
					bGotMsgForCaller = true;
					m_bPlaying = false;
					break;

				case NetMsg::START_REALM:
					{
					// Start playing
					m_bPlaying = true;
					// *SPA 12/30/97 We can send the first frame now
					m_bSendNextFrame = true;
					m_u16PackageID = 0;
					Net::ID id = 0;
					for (id = 0; id < Net::MaxNumIDs; id++)
						{
						// Only set time for peers that are joined
						if (m_aPeers[id].m_state == CPeer::Joined)
							m_aPeers[id].m_lLastReceiveTime = rspGetMilliseconds();
						}
					}
					break;

				case NetMsg::HALT_REALM:
					// Set the halt frame
					SetHaltFrame(pmsg->msg.haltRealm.seqHalt);
					break;

				case NetMsg::NEXT_REALM:
					// Set the halt frame.  The server will never (in theory) allow more
					// than one halt frame to be in effect at any time.
					SetHaltFrame(pmsg->msg.nextRealm.seqHalt);
					m_bSendNextFrame = false; 

					// Note that we don't return this message to the caller.  Instead,
					// we set this flag, which tells us to generate a NEXT_REALM for
					// the caller when we reach the halt frame.
					m_bNextRealmPending = true;
					break;

				case NetMsg::PROGRESS_REALM:
					// Whenever the server receives a READY_REALM message from a client,
					// it sends this message to all clients, telling them how many clients
					// are ready.  This is just a simple status message intended to allow
					// the app to give the user feedback about what is happening.

					// Return this message to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::PROCEED:
					// This is a simple app-level message used by the server to tell all
					// players to "proceed" to whatever the next step of the program is

					// Return this message to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::PING:
					// Calculate ping time and stuff result back into message so high
					// level has easy access to it.
//					m_lLatestPingTime = rspGetMilliseconds() - pmsg->msg.ping.lTimeStamp;
//					pmsg->msg.ping.lLatestPingResult = m_lLatestPingTime;
					break;

				case NetMsg::RAND:
					break;

				default:
					TRACE("CNetClient::GetMsg(): Unexpected message received!\n");
					break;
				}
			}
		}

	// If we have a message for the caller, use it.  Otherwise, return a NOTHING msg
	if (!bGotMsgForCaller)
		{
		// Create a nothing message
		pmsg->msg.nothing.ucType = NetMsg::NOTHING;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Send message to server
////////////////////////////////////////////////////////////////////////////////
void CNetClient::SendMsg(
	NetMsg* pmsg,											// In:  Message to send
	bool bSendNow /*= true*/)							// In:  Whether to send now or wait until Update()
	{
	// We purposefully don't check m_msgError.msg.error here because there's no
	// particular reason not to allow sending messages, since m_msgError.msg.error
	// doesn't currently get modified here, so there's no danger of overwriting a 
	// previous error.
	
	// No point in sending message if we're not connected
	if (m_msgr.GetState() == CNetMsgr::Connected)
		m_msgr.SendMsg(pmsg, bSendNow);
	}


////////////////////////////////////////////////////////////////////////////////
// Send chat message (text is sent with player's name as a prefix)
////////////////////////////////////////////////////////////////////////////////
void CNetClient::SendChat(
	const char* pszText)									// In:  Text to send
	{
	if (m_state == Joined)
		{
		// Create chat message.  For now, we set the mask to include everyone.  In
		// the future, we could allow the user to specify who will/won't get it.
		NetMsg msg;
		msg.msg.chatReq.ucType = NetMsg::CHAT_REQ;
		msg.msg.chatReq.u16Mask = 0xffff;

		// We're assuming the chat field is longer than the maximum name
		ASSERT(sizeof(msg.msg.chatReq.acText) > sizeof(m_aPeers[m_id].m_acName));

		// Calculate number of chars required to display name, including the brackets
		// and the space and the null.
		int iNameChars = strlen(m_aPeers[m_id].m_acName) + 4;

		// Calculate space remaining for chat text
		int iChatChars = sizeof(msg.msg.chatReq.acText) - iNameChars;

		// Form chat string.  Note that a maximum of iChatChars worth of chat text is printed.
		sprintf(msg.msg.chatReq.acText, "[%s] %.*s", m_aPeers[m_id].m_acName, iChatChars, pszText);

		// Send message
		SendMsg(&msg);
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Send text message (text is send as is)
////////////////////////////////////////////////////////////////////////////////
void CNetClient::SendText(
	const char* pszText)									// In:  Text to send
	{
	if (m_state == Joined)
		{
		// Create chat message.  For now, we set the mask to include everyone.  In
		// the future, we could allow the user to specify who will/won't get it.
		NetMsg msg;
		msg.msg.chatReq.ucType = NetMsg::CHAT_REQ;
		msg.msg.chatReq.u16Mask = 0xffff;

		// Copy text, truncating in case the specified text is too long
		strncpy(msg.msg.chatReq.acText, pszText, sizeof(msg.msg.chatReq.acText));
		msg.msg.chatReq.acText[sizeof(msg.msg.chatReq.acText)-1] = 0;

		// Send message
		SendMsg(&msg);
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Send realm status
////////////////////////////////////////////////////////////////////////////////
void CNetClient::SendRealmStatus(
	bool bReady)
	{
	NetMsg msg;
	if (bReady)
		{
		msg.msg.readyRealm.ucType = NetMsg::READY_REALM;
		SendMsg(&msg);
		}
	else
		{
		msg.msg.badRealm.ucType = NetMsg::BAD_REALM;
		SendMsg(&msg);
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Drop self
////////////////////////////////////////////////////////////////////////////////
void CNetClient::Drop(void)
	{
	NetMsg msg;

	switch (m_state)
		{
		case Nothing:
			// Nothing to do
			break;

		case WaitForConnect:
			// The disconnect at the end is all we need to do
			break;

		case WaitForLoginResponse:
			// We already sent a LOGIN but don't know if it'll be accepted.  If so,
			// this LOGOUT will be fine, and if not, it won't matter.
			msg.msg.logout.ucType = NetMsg::LOGOUT;
			SendMsg(&msg);
			break;

		case WaitForJoinResponse:
			// We already sent a JOIN_REQ but don't know if it'll be accepted.  The
			// safe thing to do is to send a LOGOUT, which will work either way.
			msg.msg.logout.ucType = NetMsg::LOGOUT;
			SendMsg(&msg);
			break;

		case Joined:
			// Send drop request
			msg.msg.dropReq.ucType = NetMsg::DROP_REQ;
			SendMsg(&msg);
			break;

		default:
			TRACE("CNetClient::Drop(): Unknown state!\n");
			break;
		}

	// Disconnect cleanly
	m_msgr.Disconnect(true);

	// Clear net ID
	m_id = Net::InvalidID;

	// Reset state
	m_state = Nothing;
	}


////////////////////////////////////////////////////////////////////////////////
// Receive messages from peers
////////////////////////////////////////////////////////////////////////////////
void CNetClient::ReceiveFromPeers(void)
	{

	/*** 12/10/97 AJC ***/
	// Limit the number of times we spend in the loop to the number of players 
	// Process incoming messages, but limit the maximum time we spend in the loop,
	// because if there's a ton of incoming data, we could get stuck here forever!
	// This is done as a do/while so that we can more easily debug it (otherwise,
	// when single-stepping, the time expires before we ever get into the loop!)
	//long lMaxTime = rspGetMilliseconds() + Net::MaxPeerReceiveTime;
	short sIterations = 0;
	do	{
		// Call watchdog to let it know we're still going (we're in a loop!)
		NetBlockingWatchdog();

		// Make sure maximum message size is <= maximum datagram size
		ASSERT(PEER_MSG_MAX_SIZE <= Net::MaxDatagramSize);

		// Get next datagram, If we get a too-large datagram, an error occurs and
		// and the unreceived portion is automatically discarded.  Such a message
		// could come from a foreign app that is using the same port as us.
		U8 msg[PEER_MSG_MAX_SIZE];
		long lReceived;
		short serr = m_socketPeers.ReceiveFrom(msg, sizeof(msg), &lReceived, NULL);
		if (serr == 0)
			{
			// Make sure size is within proper range
			if ((lReceived >= PEER_MSG_HEADER_SIZE) && (lReceived <= PEER_MSG_MAX_SIZE))
				{

				// Get the id from the message
				Net::ID id;
				U8* pget = msg;
				Get(pget, &id);

				// Make sure id is valid
				if ((id >= 0) && (id < Net::MaxNumIDs))
					{
					// Make sure peer is joined (could be an old message from a dropped peer)
					if (m_aPeers[id].m_state == CPeer::Joined)
						{
						// Calculate the number of input values the message contains. *SPA add sizeof frame time
						long lNumInputs = (lReceived - PEER_MSG_HEADER_SIZE) / (sizeof(UINPUT) + sizeof(U8));

						// Get the rest of the message
//						U16 u16SenderPing;
//						U16 u16ReceiverPing;
						U16 u16MsgType; // *SPA
						U16 u16PackageID; // *SPA
						Net::SEQ seqWhatHeNeeds;
						Net::SEQ seqInputs;
						Get(pget, &u16PackageID); // *SPA
						Get(pget, &u16MsgType); // *SPA
//						Get(pget, &u16SenderPing);
//						Get(pget, &u16ReceiverPing);
						Get(pget, &seqWhatHeNeeds);
						Get(pget, &seqInputs);

						// Reset the receive timer for this peer *SPA
						m_aPeers[id].m_lLastReceiveTime = rspGetMilliseconds();

/*						// 12/7/97 AJC
#ifdef WIN32
						if (g_GameSettings.m_bLogNetTime)
							{
							WriteTimeStamp("ReceiveFromPeers()", 
												m_aPeers[id].m_acName, 
												NetMsg::INPUT_DATA, 
												seqInputs,
												lNumInputs,
												true,
												u16PackageID);
							}
#endif
						// 12/7/97 AJC
*/
						// Add input values to peer's input buffer
						UINPUT input;
						U8 frameTime;
						for (short s = 0; s < lNumInputs; s++)
							{
							// Add input to peer's buffer
							Get(pget, &input);
							m_aPeers[id].m_netinput.Put(seqInputs, input);

							// Add frame time to peer's buffer *SPA
							Get(pget, &frameTime);
							m_aPeers[id].m_netinput.PutFrameTime(seqInputs++, frameTime);
							}

						// If u16MsgType is 1 then this is a reqest for data *SPA
						if (u16MsgType == 1)
							{
							// So let's send him data starting from the frame he needs now *SPA
							SendToPeer(id, seqWhatHeNeeds, false);
							}

						// Now that I got new inputs, determine the first input seq I need from him
//						m_aPeers[id].m_seqWhatINeed = m_aPeers[id].m_netinput.FindFirstInvalid();

						// Since messages can arrive in the wrong order, we want to ignore older
						// versions of this value so we don't send him more than he really needs.
						// We can safely assume that what he needs will never go backwards from what
						// he previously said -- it will stay the same or go further ahead.  So we
						// only use the new value if it's greater than what we have.
//						if (SEQ_GTE(seqWhatHeNeeds, m_aPeers[id].m_seqWhatHeNeeds))
//							m_aPeers[id].m_seqWhatHeNeeds = seqWhatHeNeeds;
#if 0
						// Add his ping time into the average
						m_aPeers[id].m_lHisPingSum += (long)u16SenderPing;
						m_aPeers[id].m_sHisNumPings++;

						// Calculate the time our ping took to get back (it won't be valid until
						// he starts getting my ping times -- before that, he'll set it to 0xffff)
						if (u16ReceiverPing != 0xffff)
							{
							}
#endif
						}
					else
						TRACE("CNetClient::ReceiveFromPeers(): Ignoring message from non-joined player!\n");
					}
				else
					TRACE("CNetClient::ReceiveFromPeers(): Ignoring message with invalid id!\n");
				}
			else
				TRACE("CNetClient::ReceiveFromPeers(): Ignoring incorrectly sized message!\n");
			}
		else
			{
			if (serr != RSocket::errWouldBlock)
				TRACE("CNetClient::ReceiveFromPeers(): Error receiving datagram -- ignored!\n");

			// Break out of the loop (there's no data or we got an error)
			break;
			}
		/*** 12/10/97 AJC ***/
		//} while (rspGetMilliseconds() < lMaxTime);
		sIterations++;
		} while (sIterations < (m_sNumJoined * 2));
		/*** 12/10/97 AJC ***/
	}


////////////////////////////////////////////////////////////////////////////////
// Send messages to peers
////////////////////////////////////////////////////////////////////////////////
// 12/30/97 *SPA Pulled center loop out to seperate routine (SendToPeer) and
//				simplified to send only one packet per frame
void CNetClient::SendToPeers(void)
	{
	// Are we ready to send the next frame
	if (m_bSendNextFrame)
		{
		m_bSendNextFrame = false;

		// Go through all the peers
		Net::ID id = 0;
		for (id = 0; id < Net::MaxNumIDs; id++)
			{
			// Only send messages to joined peers
			if (m_aPeers[id].m_state == CPeer::Joined)
				{
				// An interesting note: If the "halt frame" is in effect, we don't
				// need to explicitly check for it here.  It is checked for by the
				// function that handles the getting of local input.  That function
				// simply will not add any addition local input beyond the halt frame,
				// so we are automatically prevented from sending out anything that
				// we don't have.

				// If it is not us (we've already set our own in SetLocalInput)
				if (id != m_id)
					{
					// Send to peer starting from the first frame after the one we know he has
					// (since we have all the inputs for frame m_seqFrame - 1, everyone must have
					// rendered frame (m_seqFrame - MaxAheadSeq - 1) or else they would not have
					// sent out frame m_seqFrame.
					// Since this is just a routine send, we don't need to request a frame.
					// Make sure we don't try to send any frame before frame 0!!
					if (m_seqFrame < (m_seqMaxAhead + 1))
						SendToPeer(id, 0, false);
					else
						SendToPeer(id, m_seqFrame - m_seqMaxAhead - 1, false);
					}
				}
			}
		}
	}



////////////////////////////////////////////////////////////////////////////////
// Send message to single peer  *SPA
////////////////////////////////////////////////////////////////////////////////
void CNetClient::SendToPeer(Net::ID id,				// id of peer to send to
							Net::SEQ seqStart,		// frame to start at
							bool bSeqReq)			// true if this is a request for data
	{
	// Set the header values
	U8 msg[PEER_MSG_MAX_SIZE];
	U8* pput = msg;
	
	// Increment the package number
	m_u16PackageID++;
	Put(pput, m_id);
	Put(pput, m_u16PackageID );

	if (!bSeqReq)
		{
		Put(pput, (U16)0);
		Put(pput, (U16)0);
		}
	else
		{
		// If bSeqReq then set flag for data request
		Put(pput, (U16)1);
		// request the data for the current frame (since that must be the one that we can't render)
		Put(pput, (U16)m_seqFrame);
		}

	Put(pput, seqStart);
	ASSERT((pput - msg) == PEER_MSG_HEADER_SIZE);
	// Fill in all the inputs between what he needs and the newest I have,
	// up to the maximum amount.  We know we'll get at least 1 because
	// we already determined the one he needs is available (see above).
	UINPUT input;
	U8 frameTime; // *SPA
	short s;
	for (s = seqStart; s < m_seqInputNotYetSent; s++)
		{
		input = m_netinput.Get(s);
		// Send game time for this frame *SPA
		frameTime = m_netinput.GetFrameTime(s);
		if (input != CNetInput::Invalid)
			{
			Put(pput, input);
			Put(pput, frameTime); // *SPA
			}
		else
			break;
		}


/*	// 12/7/97 AJC
#ifdef WIN32
	if (g_GameSettings.m_bLogNetTime)
		{
		if (!bSeqReq)
			WriteTimeStamp("SendToPeer()", 
								m_aPeers[id].m_acName, 
								NetMsg::INPUT_DATA, 
								seqStart,
								s - seqStart,
								false,
								m_u16PackageID);
		else
			WriteTimeStamp("SendFrameRequest", 
								m_aPeers[id].m_acName, 
								NetMsg::INPUT_DATA, 
								seqStart,
								s - seqStart,
								false,
								m_u16PackageID);
		}
#endif
	// 12/7/97 AJC
*/

	// Calculate size of message.  The peer uses the message size to determine
	// how many input values it contains.
	ASSERT((pput - msg) <= PEER_MSG_MAX_SIZE);
	long lSize = pput - msg;

	// Make sure maximum message size is <= maximum datagram size
	ASSERT(PEER_MSG_MAX_SIZE <= Net::MaxDatagramSize);

	// Send message to peer.  If an error occurs, I'm going to ignore it in the
	// hopes that the next time we try, it will work.  This may not be a good
	// idea.  Although datagram messages are not guaranteed to arrive, getting
	// a send error probably indicates a real problem that may not go away.
	long lSent;
	short serr = m_socketPeers.SendTo(msg, lSize, &lSent, &m_aPeers[id].m_address);
	if (serr == 0)
		{
		if (lSent != lSize)
			TRACE("Error sending message to peer -- should have sent %ld bytes but actually sent %ld.\n", (long)lSize, (long)lSent);
		}
	else
		{
		if (serr != RSocket::errWouldBlock)
			TRACE("Error sending message to peer -- SendTo() failed!\n");
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Determine whether another frame can be done.  If so, the necessary peer
// inputs are returned in the supplied array and the result is true.
// Otherwise, the result is false, and a frame cannot be done.
////////////////////////////////////////////////////////////////////////////////
bool CNetClient::CanDoFrame(							// Returns true if frame can be done, false otherwise
	UINPUT aInputs[],										// Out: Total of Net::MaxNumIDs inputs returned here
	short* psFrameTime)									// Out the current frames elapsed time
	{
	bool bResult = false;
	long		lFrameTime = 0; // The sum of the frame times of the joined players *SPA
	short		sCount = 0;		// Count of the number of joined players *SPA

	// If we playing, we might be able to do this, otherwise, we definitely can't
	if (m_bPlaying)
		{

		// Try to get the required input seq from each peer
		bResult = true;
		for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
			{
			if (m_aPeers[id].m_state == CPeer::Joined)
				{
				// Try to get the input for the frame we're trying to do
				aInputs[id] = m_aPeers[id].m_netinput.Get(m_seqFrame);
				long temp = m_aPeers[id].m_netinput.GetFrameTime(m_seqFrame); // *SPA
				lFrameTime += temp; // *SPA
				sCount++;

				if (aInputs[id] == CNetInput::Invalid)
					{
					bResult = false;
					break;
					}
				}
			else if (m_aPeers[id].m_state == CPeer::Dropped)
				{
				// If dropped peer is inactive, always return "suicide", otherwise
				// continue returning what is left of his inputs.
				if (m_aPeers[id].m_bInactive)
					{
					aInputs[id] = INPUT_SUICIDE;
					}
				else
					{
					// Get the next input -- note that a dropped player should NEVER return
					// an invalid value, because there's no way to figure out what it
					// should be now that he's dropped!
					aInputs[id] = m_aPeers[id].m_netinput.Get(m_seqFrame);
					ASSERT(aInputs[id] != CNetInput::Invalid);
					}
				}
			}

		// Check to see if we've not been able to render this frame for a while *SPA
		long lCurTime = rspGetMilliseconds();
		if (lCurTime > m_lMaxWaitTime)
			{
			for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
				{
				if ((m_aPeers[id].m_state == CPeer::Joined) && (id != m_id))
					{
					// Send a request for the current frame data from each peer that we don't have
					// This also resends the last data we sent, just in case it needs it before it
					// can send what we need
					aInputs[id] = m_aPeers[id].m_netinput.Get(m_seqFrame);
					if (aInputs[id] == CNetInput::Invalid)
						{
						// Make sure we don't try to send any frame before frame 0!!
						if (m_seqFrame < (m_seqMaxAhead + 1))
							SendToPeer(id, 0, true);
						else
							SendToPeer(id, m_seqFrame - m_seqMaxAhead - 1, true);
						}
					}
				}
			// Reset timer
			m_lMaxWaitTime = rspGetMilliseconds() + g_GameSettings.m_sNetSendInputInterval;
			}
		// *SPA

		// If we have everything we need to go to the next frame, do it now
		if (bResult)
			{
			// Let SendToPeers know we have a frame to render
			m_bSendNextFrame = true;
			// Set our max time out for the next frame
			m_lMaxWaitTime = rspGetMilliseconds() + g_GameSettings.m_sNetSendInputInterval;
			// Calculate frame time of the frame were about to render *SPA
			// This is the average time for the current frame
			m_alAvgFrameTimes[m_seqFrame & 0x7] = lFrameTime / sCount;
			long lAvgTime = 0;
			// This averages the frame times of the last 8 frames
			for (short i = 0; i< 8; i++)
				{
				lAvgTime += m_alAvgFrameTimes[i];
				}
			*psFrameTime = lAvgTime / 8;

			/** 12/15/97 *SPA **/
			m_lFrameTime = *psFrameTime;

			// Calculate the time since the last time here *SPA
//			long lCurTime = rspGetMilliseconds();
			long frameTime = lCurTime - m_lStartTime;
			m_lStartTime = lCurTime;
			// Limit our frame rate to minimum set by .ini (TimePerFrame) *SPA
			if (frameTime > g_GameSettings.m_sNetTimePerFrame)
				{
				frameTime = g_GameSettings.m_sNetTimePerFrame;
				}
			// Put this time into the next frame that we know has not been sent to anybody *SPA
			m_netinput.PutFrameTime(m_seqInputNotYetSent, (U8)(frameTime & 0x00ff));

			// This is the only place where we increment the frame seq, so it's the
			// right place to check if we've reached the halt frame (if there is one).
			// If we reach it, stop playing.  Remember, it's okay to actually DO this
			// frame, we just can't do the frame beyond it, and m_seqFrame always
			// indicates the frame it is TRYING to do, not the frame it already did,
			// so if m_seqFrame ends up one BEYOND m_seqHaltFrame, it's perfect!
			if (m_bUseHaltFrame && (m_seqFrame == m_seqHaltFrame))
				{
				// We're at the halt frame, so stop
				m_bReachedHaltFrame = true;
				m_bPlaying = false;
				}
			// Increment actual frame seq
			m_seqFrame++;
			m_seqInputNotYetSent++;	// *SPA

			// Increment my net input buffer
			m_netinput.IncFrame();

			// Handle the peer input buffers
			for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
				{
				if (m_aPeers[id].m_state == CPeer::Joined)
					{
					// For joined players, just inc it
					m_aPeers[id].m_netinput.IncFrame();
					}
				else if (m_aPeers[id].m_state == CPeer::Dropped)
					{
					// If dropped player is still active, check if he's hit his final active seq
					if (!m_aPeers[id].m_bInactive)
						{
						// If his frame is currently on the final active seq, then he's done.  We
						// know that the frame he's currently on is the one that was just returned
						// to the caller, so it makes sense that if we just used his last active
						// seq, he should now become inactive.  If he hasn't reached his final seq,
						// then we just inc his frame.
						if (m_aPeers[id].m_netinput.GetFrameSeq() == m_aPeers[id].m_seqLastActive)
							m_aPeers[id].m_bInactive = true;
						else
							m_aPeers[id].m_netinput.IncFrame();
						}
					}
				}
			}
		}

	return bResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Check whether local input is required
////////////////////////////////////////////////////////////////////////////////
bool CNetClient::IsLocalInputNeeded(void)
	{
	bool bResult = false;

	// Only if we're currently playing
	if (m_bPlaying)
		{
		// Check if timer expired
//		long lCurTime = rspGetMilliseconds();
//		if (lCurTime > m_lNextLocalInputTime)
//			{
			// The input seq is only allowed to get m_seqMaxAhead ahead of the
			// frame seq.  Both m_seqInput and m_seqFrame are really refering to
			// the NEXT seq.  When we say we're "on frame 0", we really mean we're
			// waiting for all the inputs for frame 0 to arrive.  As soon as they
			// do, we increment to frame 1 and start waiting again.  The input
			// seq is the same way -- it really indicates the seq that we're GOING
			// TO GET.  Therefore, we can let the input seq get m_seqMaxAhead
			// ahead of the frame seq, but we DON'T actually get local input when it's
			// at that point.  Only when the frame moves up will we get local input
			// for that position.
			// SPA 12/30/97 Changed m_seqMaxAhead to m_seqMaxAhead+1 (we were getting
			// one less ahead than we thought we were)
			ASSERT((Net::SEQ)(m_seqInput - m_seqFrame) <= (m_seqMaxAhead+1));
			if ((Net::SEQ)(m_seqInput - m_seqFrame) < (m_seqMaxAhead+1))
				{
				// Don't send out any inputs BEYOND the stop seq.  CanDoFrame() will
				// let us DO the m_seqHalt frame, so we need to send out inputs up to
				// and including that frame, but not beyond it.  This will allow
				// everyone to DO that frame, just like we did, but they will not be
				// able to go beyond it (certainly not without our input, if nothing
				// else).  
				if ( !(m_bUseHaltFrame && SEQ_GT(m_seqInput, m_seqHaltFrame)) )
					{
					// We need local input!
					bResult = true;

					// Don't reset the timer until we have actually decided we need
					// local input.  The idea is that if we can't move the input
					// ahead for whatever reason, then the instant we CAN move ahead
					// we want to do so.  By waiting until here to reset the timer,
					// we ensure that if we don't get here, it will remain expired.
//					m_lNextLocalInputTime = lCurTime + m_lFrameTime;
					}
				else if (m_bUseHaltFrame)
					{
					UINPUT input = m_netinput.Get(m_seqFrame);
					m_aPeers[m_id].m_netinput.Put(m_seqFrame, input);
					U8 frameTime = m_netinput.GetFrameTime(m_seqFrame);
					m_aPeers[m_id].m_netinput.PutFrameTime(m_seqFrame, frameTime);
					}
				}
//			}
		}

	return bResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Set local input.
// Call this if and only if IsLocalInputNeeded() returns true!
////////////////////////////////////////////////////////////////////////////////
void CNetClient::SetLocalInput(
	UINPUT input)
	{
	// Add to buffer and increment input seq
	m_netinput.Put(m_seqInput, input);
	m_seqInput++;

	if (m_bPlaying && m_id != Net::InvalidID)
		{
		// Set our own peer data for the current frame from the local data *SPA 12/30/97
		input = m_netinput.Get(m_seqFrame);
		m_aPeers[m_id].m_netinput.Put(m_seqFrame, input);
		U8 frameTime = m_netinput.GetFrameTime(m_seqFrame);
		m_aPeers[m_id].m_netinput.PutFrameTime(m_seqFrame, frameTime);
		// Reset the receive timer *SPA
		m_aPeers[m_id].m_lLastReceiveTime = rspGetMilliseconds();
		}
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
void CNetClient::SetHaltFrame(
	Net::SEQ seqHalt)
	{
	// This sets the last input seq we are allowed to send to any peers,
	// including ourself!  This effectively halts all the peers, including
	// ourself, when they reach the specified frame, because they will not
	// have any inputs from us beyond that frame.

	// Set the frame to halt on and set the flag so we'll check for it
	m_seqHaltFrame = seqHalt;
	m_bUseHaltFrame = true;

	// I'm not sure this can happen, but it's certainly worth checking.  The
	// halt frame is presumably a frame we haven't done yet.  And remember
	// that m_seqFrame indicates the frame we're trying to do, not the frame
	// we already did.  If, for some reason, we are sent a halt frame that
	// is LESS than m_seqFrame, then it means something has gone very wrong.
	// So let's make sure that the halt frame is always greater than or equal
	// to m_seqFrame.
	ASSERT(SEQ_GTE(m_seqHaltFrame, m_seqFrame));
	}

////////////////////////////////////////////////////////////////////////////////
// This is used to see if any peers have not sent data for too long of a period
//
// It returns the id of the first peer it finds that has exceeded the time limit
// so that the server can drop him. It should only be called if this client is
// attached to the server. If more than one peer has exceeded the time limit,
// it will be handled on a subsequent pass. By then the first late peer will
// have been marked as dropped.
// Returns Net::MaxNumIDs if no peer has exceeded time limit 1/13/98 *SPA
////////////////////////////////////////////////////////////////////////////////
Net::ID CNetClient::CheckForLostPeer(void)
	{
	if (m_bPlaying)
		{
		for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
			{
			if (m_aPeers[id].m_state == CPeer::Joined)
				{
				if (id != m_id)
					{
					long lElapsedTime = rspGetMilliseconds() - m_aPeers[id].m_lLastReceiveTime;
					if (lElapsedTime > g_GameSettings.m_lPeerDropMaxWaitTime)
						return id;
					}
				}
			}
		}
	return Net::MaxNumIDs;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////