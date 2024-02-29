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
// NetServer.cpp
// Project: RSPiX
//
// History:
//		09/01/97 MJR	Nearing the end of a major overhaul.
//
//		09/06/97 MJR	No longer responds to browse attempts once game starts.
//
//		09/07/97 MJR	Added support for PROCEED and PROGRESS_REALM messages.
//
//		11/20/97	JMI	Added support for new sCoopLevels & sCoopMode flag in 
//							StartGame and SetupGame messages.
//
//		11/25/97	JMI	Added determination between version conflicts and platform
//							conflicts.  Also, added error propagation.
//
//		11/26/97	JMI	Masking error in evaluation of version mismatch problem
//							such that platform mismatch was never detected.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "netserver.h"


////////////////////////////////////////////////////////////////////////////////
// Startup
////////////////////////////////////////////////////////////////////////////////
short CNetServer::Startup(								// Returns 0 if successfull, non-zero otherwise
	unsigned short usPort,								// In:  Server base port number
	char* pszHostName,									// In:  Host name (max size is MaxHostName!!!)
	RSocket::BLOCK_CALLBACK callback)				// In:  Blocking callback
	{
	short sResult = 0;

	// Do a reset to be sure we're starting at a good point
	Reset();

	// Save base socket (all sockets are done as offsets from this one)
	m_usBasePort = usPort;

	// Save callback
	m_callback = callback;

	// Create a unique number that this host can use to identify itself when
	// it browses for hosts.  We simply use total elapsed microseconds, which
	// is EXTREMELY LIKELY to be different for every host.  We also look for
	// the correct host name, but of course, many hosts can have the same name.
	m_lHostMagic = rspGetMicroseconds();

	// Save name (truncating if necessary)
	strncpy(m_acHostName, pszHostName, Net::MaxHostNameSize);
	m_acHostName[Net::MaxHostNameSize-1] = 0;

	// Setup listen socket
	sResult = m_socketListen.Open(
		m_usBasePort + Net::ListenPortOffset,
		RSocket::typStream,
		RSocket::optDontWaitOnClose | RSocket::optDontCoalesce | RSocket::optDontBlock,
		callback);
	if (sResult == 0)
		{
		sResult = m_socketListen.Listen();
		if (sResult == 0)
			{

			// Setup antenna socket, on which we receive broadcasts from potential
			// clients browsing for a host.
			sResult = m_socketAntenna.Open(
				m_usBasePort + Net::AntennaPortOffset,
				RSocket::typDatagram,
				RSocket::optDontBlock,
				callback);
			if (sResult == 0)
				{
				// Must set broadcast mode even for sockets that are RECEIVING them.  Doesn't
				// seem to make sense, but empericial results say we need to do this.
				sResult = m_socketAntenna.Broadcast();
				if (sResult == 0)
					{

					}
				else
					TRACE("CNetServer::Setup(): Error putting antenna socket into broadcast mode!\n");
				}
			else
				TRACE("CNetServer::Setup(): Error opening antenna socket!\n");
			}
		else
			TRACE("CNetServer::Setup(): Error putting listen socket into listen mode!\n");
		}
	else
		TRACE("CNetServer::Setup(): Error opening listen socket!\n");

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown
////////////////////////////////////////////////////////////////////////////////
void CNetServer::Shutdown(void)
	{
	Reset();
	}


////////////////////////////////////////////////////////////////////////////////
// Update server.  This should be called regularly.
////////////////////////////////////////////////////////////////////////////////
void CNetServer::Update(void)
	{
	short sResult = 0;

	//------------------------------------------------------------------------------
	// Accept new clients
	//------------------------------------------------------------------------------

	// If there's no error on the listen socket, we can try to accept clients
	if (!m_socketListen.IsError())
		{
		// Look for an unconnected, unused slot in the array.  I'm torn between searching
		// for an empty client each time versus checking if the socket can accept without
		// blocking each time -- I'm not sure which one is quicker!  Obviously, the quicker
		// of the two should be checked first.
		for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
			{
			if ((m_aClients[id].m_msgr.GetState() == CNetMsgr::Disconnected) && (m_aClients[id].m_state == CClient::Unused))
				{
				// Check if a new client is trying to connect
				if (m_socketListen.CanAcceptWithoutBlocking())
					{
					// Try to accept client's connection
					short serr = m_aClients[id].m_msgr.Accept(&m_socketListen, m_callback);
					if (serr == 0)
						{
						// Upgrade state
						m_aClients[id].m_state = CClient::Used;
						}
					else
						{
						// If the return error indicates that it would have blocked, then something
						// is not kosher since CanAcceptWithoutBlocking() just said it woulnd NOT block.
						if (serr == RSocket::errWouldBlock)
							TRACE("CNetServer()::Update(): It waid it wouldn't block, but then said it would!\n");

						// Don't return an actual error code from this function because we can't
						// get all bent-out-of-shape over not being able to connect to a client
//						TRACE("CNetServer()::Update(): Tried to accept connection, but failed.\n");
						}
					}
				// Break out of loop that was looking for unused clients
				break;
				}
			}
		}

	//------------------------------------------------------------------------------
	// Check for any reception on our antenna socket.  Potential clients that are
	// looking for a host will periodically broadcast a message, and as a server
	// we are supposed to respond.
	//------------------------------------------------------------------------------

	// Don't respond once the game has started because we currently don't support
	// drop-in play, so we don't want to confuse potential players
	if (!m_bGameStarted)
		{
		// Try to read a message.  This will almost always fail due to a lack of data,
		// since it's relatively rare for a client to be browsing, and even then it only
		// sends out a message every so often.  If we get an incorrectly-sized message,
		// we simply ignore it -- this is a datagram socket, so if the message was larger
		// than we expected, the rest of it will be discarded, and if it was smaller, then
		// we can ignore it as well.  Bad messages could come from a foreign app that is
		// using the same port as us.  If we do get a message, the address of the sender
		// will be recorded -- this gives us the host's address!
		U8 buf1[4];
		long lReceived;
		RSocket::Address address;
		short serr = m_socketAntenna.ReceiveFrom(buf1, sizeof(buf1), &lReceived, &address);
		if (serr == 0)
			{
			// Validate the message to make sure it was sent by another app of this
			// type, as opposed to some unknown app that happens to use the same port.
			if ((lReceived == sizeof(buf1)) &&
				 (buf1[0] == Net::BroadcastMagic0) &&
				 (buf1[1] == Net::BroadcastMagic1) &&
				 (buf1[2] == Net::BroadcastMagic2) &&
				 (buf1[3] == Net::BroadcastMagic3))
				{
				// Send back a message including the host name and magic number
				// The endian nature of the magic number will always be correct because
				// the only entitity that is meant to recognize this value is the one
				// that sent it, so as long as the encoding and decoding of the bytes
				// is the same, that entity will get the same value that it sent.  All
				// other entities will see this as a meaningless value, which is fine.
				U8 buf2[4 + 4 + sizeof(m_acHostName)];
				buf2[0] = Net::BroadcastMagic0;
				buf2[1] = Net::BroadcastMagic1;
				buf2[2] = Net::BroadcastMagic2;
				buf2[3] = Net::BroadcastMagic3;
				buf2[4] = (U8)(m_lHostMagic & 0xff);
				buf2[5] = (U8)((m_lHostMagic >>  8) & 0xff);
				buf2[6] = (U8)((m_lHostMagic >> 16) & 0xff);
				buf2[7] = (U8)((m_lHostMagic >> 24) & 0xff);
				strncpy((char*)&buf2[8], m_acHostName, sizeof(m_acHostName));

				// Send the message directly to the sender of the previous message
				long lBytesSent;
				short serr = m_socketAntenna.SendTo(buf2, sizeof(buf2), &lBytesSent, &address);
				if (serr == 0)
					{
					if (lBytesSent != sizeof(buf2))
						TRACE("CNetServer::Update(): Error sending broadcast (wrong size)!\n");
					}
				else
					{
					if (serr != RSocket::errWouldBlock)
						TRACE("CNetServer::Update(): Error sending broadcast!\n");
					}
				}
			else
				TRACE("CNetServer::Update(): Validation failed -- another app may be sending crap to our port!\n");
			}
		else
			{
			if (serr != RSocket::errWouldBlock)
				TRACE("CNetServer::Update(): Warning: Error receiving broadcast -- ignored!\n");
			}
		}

	//------------------------------------------------------------------------------
	// Update clients
	//------------------------------------------------------------------------------
	for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
		m_aClients[id].m_msgr.Update();
	}


////////////////////////////////////////////////////////////////////////////////
// Get message
////////////////////////////////////////////////////////////////////////////////
void CNetServer::GetMsg(
	NetMsg* pmsg)											// Out: Message is returned here
	{
	// This indicates whether we got a message to be returned to the caller
	bool bGotMsgForCaller = false;

	// We need to service all of the currently connected clients, but we can
	// only handle one message each time this function is called.  Actually,
	// we can only handle one message if it's a message that needs to be
	// returned to the caller.  If it wasn't, then we could theoretically
	// handle many such messages, with the only limitation being how much time
	// we're willing to spend here.  For now, I'm going with handling just one
	// message each time.
	//
	// In order to give each client a chance, we use m_idPrevGet to keep track
	// of which client we got a message from the previous time.  Starting with
	// the next client after that, we search for the first client that is
	// connected and has a message, and we process it.  We save that client's
	// id so that next time we will start with the client after it.
	//
	// If we go through the entire list once and don't find any clients with
	// messages to get, then we have no client messages to process this time.
	bool bGotOne = false;
	Net::ID id = m_idPrevGet;
	do	{
		if (++id >= Net::MaxNumIDs)
			id = 0;

		if (m_aClients[id].m_msgr.GetState() == CNetMsgr::Connected)
			bGotOne = m_aClients[id].m_msgr.GetMsg(pmsg);

		} while (!bGotOne && (id != m_idPrevGet));
	m_idPrevGet = id;

	// If we got a message, handle it
	if (bGotOne)
		{
		// The message's ucSenderID is not transmitted as part of the message in
		// order to save bandwidth (every byte counts :) since we know the sender
		// (we're connected to the sender) and its ID is implied by it's index.
		pmsg->ucSenderID = id;

		// Used for sending messages
		NetMsg msg;

		// The login message is the only mechanism we're using to detect foreign
		// apps that might accidentally connect to us.  None of our other messages
		// has any protection, so random data that happens to resemble a message
		// could really screw us up.  Therefore, it is very important that if the
		// first thing we recieve from a client is NOT a LOGIN message, we
		// immediately disconnect it.  And if we do get what looks like a LOGIN
		// message, we must make sure the magic numbers are right.
		if (!m_aClients[id].m_bLoggedIn)
			{
			if (pmsg->msg.nothing.ucType == NetMsg::LOGIN)
				{
				if (pmsg->msg.login.ulMagic == CNetMsgr::MagicNum)
					{
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
					////////////////////////////////////////////////////////////////

					// Check version number to see if we can support it (this also is a platform check).
					if ( (pmsg->msg.login.ulVersion >= CNetMsgr::MinVersionNum) && (pmsg->msg.login.ulVersion <= CNetMsgr::CurVersionNum))
						{
						// Compatible.
						}
					else
						{
						// Must set the type.
						pmsg->msg.err.ucType	= NetMsg::ERR;
						pmsg->ucSenderID		= Net::InvalidID;

						// Determine error type (possibilities are incompatible 
						// versions and/or incompatible platforms).
						if ( (pmsg->msg.login.ulVersion & CNetMsgr::MacVersionBit) ^ (CNetMsgr::MinVersionNum & CNetMsgr::MacVersionBit) )
							{
							// One of us is a Mac and one is a PC.
							pmsg->msg.err.error = NetMsg::ServerPlatformMismatchError;
							}
						else
							{
							// Store version number from original message before we clobber it
							// by creating an error message in the same area (not sure if this
							// is really an issue but just in case).
							ULONG	ulVersion	= pmsg->msg.login.ulVersion;
							// Incompatible version number.
							pmsg->msg.err.error		= NetMsg::ServerVersionMismatchError;
							pmsg->msg.err.ulParam	= ulVersion & ~CNetMsgr::MacVersionBit;
							}

						// Return this error msg to caller
						bGotMsgForCaller = true;

						// We rely on the client to reject us but we report the error with this
						// opportunity.
						}

					// Note that we always allow the login, even if the version number and/or
					// platform are wrong -- see comment above.
					// Allow login, responding with our version number and the client's assigned ID
					msg.msg.loginAccept.ucType = NetMsg::LOGIN_ACCEPT;
					msg.msg.loginAccept.idAssigned = id;
					msg.msg.loginAccept.ulMagic = CNetMsgr::MagicNum;
					msg.msg.loginAccept.ulVersion = CNetMsgr::CurVersionNum;
					SendMsg(id, &msg);

					// Mark client as "logged in"
					m_aClients[id].m_bLoggedIn = true;
					}
				else
					{
					// Drop client
					DropClient(id);
					}
				}
			else
				{
				// Drop client
				DropClient(id);
				}
			}
		else
			{
			// Process message
			switch(pmsg->msg.nothing.ucType)
				{
				case NetMsg::NOTHING:
					break;

				case NetMsg::STAT:
					break;

				case NetMsg::ERR:
					// If a local client has been registered, and if this error came from that client,
					// then we abort the game, because we can't continue without the local client
					// (it seems unlikely that the user would want to sit around watching a blank screen).
					// If no local client was registered, it will be invalid, and this compare will fail.
					// If it isn't a local client, then we just drop the client that reported the error.
					if (id == m_idLocalClient)
						AbortGame(NetMsg::ErrorAbortedGame);
					else
						DropClient(id);

					// Return this msg to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::LOGOUT:
					// The client will send a LOGOUT if, after having it's LOGIN request accepted, it
					// determines that the server's version number is not acceptable.  It may also
					// send this if it needs to abort at any time after it has sent a JOIN_REQ but
					// before it has received a response.  In all cases, we simply call DropClient(),
					// which knows how to handle any situation.
					DropClient(id);
					break;

				case NetMsg::JOIN_REQ:
					if (!m_bGameStarted)
						{
						// Upgrade client's state
						m_aClients[id].m_state = CClient::Joined;

						// Add client info to database.  Note that client's "peer address" is the same
						// as the one we're connected to, but with a different port number.
						m_aClients[id].m_address	= m_aClients[id].m_msgr.GetAddress();
						RSocket::SetAddressPort(m_usBasePort + Net::FirstPeerPortOffset + id, &m_aClients[id].m_address);
						memcpy(m_aClients[id].m_acName, pmsg->msg.joinReq.acName, sizeof(m_aClients[id].m_acName));
						m_aClients[id].m_ucColor		= pmsg->msg.joinReq.ucColor;
						m_aClients[id].m_ucTeam			= pmsg->msg.joinReq.ucTeam;
						m_aClients[id].m_sBandwidth	= pmsg->msg.joinReq.sBandwidth;

						// Tell client he was accepted
						msg.msg.joinAccept.ucType = NetMsg::JOIN_ACCEPT;
						SendMsg(id, &msg);

						// Send all clients (including new one) info about the new client
						msg.msg.joined.ucType		= NetMsg::JOINED;
						msg.msg.joined.id				= id;
						msg.msg.joined.address		= m_aClients[id].m_address;
						memcpy(msg.msg.joined.acName, m_aClients[id].m_acName, sizeof(msg.msg.joined.acName));
						msg.msg.joined.ucColor		= m_aClients[id].m_ucColor;
						msg.msg.joined.ucTeam		= m_aClients[id].m_ucTeam;
						msg.msg.joined.sBandwidth	= m_aClients[id].m_sBandwidth;
						SendMsg(&msg);

						// Send new client info about other joined clients
						for (Net::ID id2 = 0; id2 < Net::MaxNumIDs; id2++)
							{
							if ((id2 != id) && (m_aClients[id2].m_state == CClient::Joined))
								{
								msg.msg.joined.ucType = NetMsg::JOINED;
								msg.msg.joined.id = id2;
								msg.msg.joined.address = m_aClients[id2].m_address;
								memcpy(msg.msg.joined.acName, m_aClients[id2].m_acName, sizeof(msg.msg.joined.acName));
								msg.msg.joined.ucColor = m_aClients[id2].m_ucColor;
								msg.msg.joined.ucTeam = m_aClients[id2].m_ucTeam;
								msg.msg.joined.sBandwidth = m_aClients[id2].m_sBandwidth;
								SendMsg(id, &msg);
								}
							}

						// Send new client info about current game setup (if we know it yet!)
						if (m_bSetupGameValid)
							SendMsg(id, &m_msgSetupGame);

						// Return this msg to caller
						bGotMsgForCaller = true;
						}
					else
						{
						// Deny join request because we don't allow joins after game has started
						msg.msg.joinDeny.ucType = NetMsg::JOIN_DENY;
						msg.msg.joinDeny.ucReason = NetMsg::GameAlreadyStarted;
						SendMsg(id, &msg);

						// Drop client
						DropClient(id);
						}
					break;

				case NetMsg::CHANGE_REQ:
					// Change client's info in database
					memcpy(m_aClients[id].m_acName, pmsg->msg.changed.acName, sizeof(m_aClients[id].m_acName));
					m_aClients[id].m_ucColor		= pmsg->msg.changed.ucColor;
					m_aClients[id].m_ucTeam			= pmsg->msg.changed.ucTeam;

					// Send CHANGED message to all clients, including the one that requested the change
					msg.msg.changed.ucType	= NetMsg::CHANGED;
					msg.msg.changed.id = id;
					memcpy(msg.msg.changed.acName, m_aClients[id].m_acName, sizeof(msg.msg.changed.acName));
					msg.msg.changed.ucColor	= m_aClients[id].m_ucColor;
					msg.msg.changed.ucTeam	= m_aClients[id].m_ucTeam;
					SendMsg(&msg);

					// Return this msg to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::DROP_REQ:
					// Drop the client -- DropClient() knows how to handle every situation
					DropClient(id);
					break;

				case NetMsg::DROP_ACK:
					// If we're doing a drop, handle this, otherwise ignore it
					if (!m_qDropIDs.IsEmpty())
						{
						// Make sure we only get one per client
						ASSERT(!m_aClients[id].m_bSentDropAck);

						// Set flag indicating that this client sent the message
						m_aClients[id].m_bSentDropAck = true;

						// Save info from message
						m_aClients[id].m_seqHighestDropeeInput = pmsg->msg.dropAck.seqLastDropeeInput;
						m_aClients[id].m_seqLastDoneFrame = pmsg->msg.dropAck.seqLastDoneFrame;

						// Check if all joined clients sent a DROP_ACK
						Net::ID id2;
						for (id2 = 0; id2 < Net::MaxNumIDs; id2++)
							{
							if ((m_aClients[id2].m_state == CClient::Joined) && (!m_aClients[id2].m_bSentDropAck))
								break;
							}
						if (id2 == Net::MaxNumIDs)
							{
							// Do the messy work required to figure out if any clients needs inputs from
							// other clients.  The end result of this function is that if it needed input
							// data from a client, it asked for it and returns true.  If not, it returns
							// false, which means we can immediately go on to the final step.
							if (GotAllDropAcks() == false)
								{
								// Finish the process of dropping client.  If there's more clients in the drop
								// queue, this could also start the dropping of the next victim
								FinishDroppingClientDuringGame();
								}
							}
						}
					break;

				case NetMsg::INPUT_DATA:
					ASSERT(m_bWaitingForInputData);
					if (m_bWaitingForInputData)
						{
						// Make sure it's for the correct ID
						ASSERT(m_qDropIDs.PeekAtFront() == pmsg->msg.inputData.id);

						// No longer waiting for input data
						m_bWaitingForInputData = false;

						// Go through all the clients and feed inputs to those that need it.  Note that
						// we simply forward the message we received.  It may contain more data than
						// each client needs, but what the hell.  This could be tuned some day to
						// be a bit more efficient by only sending as much as each client needs.
						for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
							{
							if (m_aClients[id].m_state == CClient::Joined)
								{
								// If this client's highest dropee input is <= the highest frame, then he needs inputs
								if (SEQ_LT(m_aClients[id].m_seqHighestDropeeInput, m_seqHighestDoneFrame))
									SendMsg(id, pmsg);
								}
							}

						// Finish the process of dropping client.  If there's more clients in the drop
						// queue, this could also start the dropping of the next victim
						FinishDroppingClientDuringGame();
						}
					break;

				case NetMsg::CHAT_REQ:
					// Send chat text to the clients specified by the mask
					msg.msg.chat.ucType	= NetMsg::CHAT;
					strncpy(msg.msg.chat.acText, pmsg->msg.chatReq.acText, sizeof(msg.msg.chat.acText) );
					SendMsg(pmsg->msg.chatReq.u16Mask, &msg);

					// Return this msg to caller
					bGotMsgForCaller = true;
					break;

				case NetMsg::ABORT_GAME:
					break;

				case NetMsg::READY_REALM:
					// If not waiting for realm status messages, then ignore this
					if (m_bWaitingForRealmStatus)
						{
						// Make sure we only get one per client
						ASSERT(!m_aClients[id].m_bSentReadyRealm);

						// Set flag indicating that this client sent the message
						m_aClients[id].m_bSentReadyRealm = true;

						// Check if all joined clients sent a READY_REALM.  Along the way, we need to
						// count the number of clients that have sent the message so far and we need
						// to create a mask that includes only those clients that sent the message.
						// These values are used to generate the PROGRESS_REALM message afterwards.
						Net::ID id2;
						short sNumExpected = 0;
						short sNumReady = 0;
						U16 mask = 0;
						for (id2 = 0; id2 < Net::MaxNumIDs; id2++)
							{
							mask = (mask >> 1) & 0x7fff;
							if (m_aClients[id2].m_state == CClient::Joined)
								{
								sNumExpected++;
								if (m_aClients[id2].m_bSentReadyRealm)
									{
									sNumReady++;
									mask |= 0x8000;
									}
								}
							}

// This had to be disabled.  The idea was great, with one major flaw -- what if the server is
// the slow machine, so while it's off loading a level or something, all the clients have already
// sent their READY_REALM messages and haven't gotten heard anything back from the server!?!?!
// Fixing this would require that the server be called at all times, and that's very hard since
// much of what the server does occurs on the GetMsg() call, which is difficult for the app to
// deal with at every level of the code.  It would be slightly easier for the app if all this work
// were done silently by the Update() function, which is possible, but certainly not a trival
// change at this level.  By the way, another problem is that once the server DOES start getting
// called again, it will have to deal with ALL of these messages at once!  Very sucky.
#if 0
						// Send PROGRESS_REALM message to all joined clients that have already responded
						// with their own READY_REALM messages.  We tell each of them how many other
						// clients we're still waiting on.  The idea behind using the mask is that it
						// cuts down on the number of messages we sent.  We don't really want to send
						// them to clients that haven't responded yet since they are obviously busy doing
						// something else, and besides, with 16 clients, we would send 16 * 16 = 256
						// messages, which is a lot, but by cutting it down using the mask, we end up
						// with only 16+15+14+...+3+2+1 = 136, which is much better.
						msg.msg.progressRealm.ucType = NetMsg::PROGRESS_REALM;
						msg.msg.progressRealm.sNumReady = sNumReady;
						msg.msg.progressRealm.sNumNotReady = sNumExpected - sNumReady;
						SendMsg(mask, &msg);
#endif

						// If we got through the whole list, then we got all the READY_REALM messages we need
						if (sNumReady == sNumExpected)
							{
							// Send START_REALM message to all joined clients
							msg.msg.startRealm.ucType = NetMsg::START_REALM;
							SendMsg(&msg);

							// Reset all related flags
							m_bWaitingForRealmStatus = false;
							for (Net::ID id2 = 0; id2 < Net::MaxNumIDs; id2++)
								m_aClients[id2].m_bSentReadyRealm = false;
							}
						}
					break;

				case NetMsg::BAD_REALM:
					// If not waiting for realm status messages, then ignore this
					if (m_bWaitingForRealmStatus)
						{
						// Reset all related flags
						m_bWaitingForRealmStatus = false;
						for (Net::ID id2 = 0; id2 < Net::MaxNumIDs; id2++)
							m_aClients[id2].m_bSentReadyRealm = false;

						// Abort game due to error
						AbortGame(NetMsg::ErrorAbortedGame);
						}
					break;

//						case FINISH_REALM:
//							break;

				case NetMsg::PING:
					// Simply echo the ping right back, and record the latest result
					SendMsg(id, pmsg);
//					m_lLatestPingTime = pmsg->msg.ping.lLatestPingTime;
					break;

				default:
					break;
				}
			}
		}

	// If we don't already have a message for the caller, check for listen error
	// (this has lower priority than a client message because accepting new clients
	// isn't as important as servicing existing ones)
	if (!bGotMsgForCaller)
		{
		if (m_socketListen.IsError())
			{
			// Generate a listen error message
			pmsg->msg.err.ucType = NetMsg::ERR;
			pmsg->msg.err.error = NetMsg::ListenError;
			pmsg->ucSenderID = Net::InvalidID;
			bGotMsgForCaller = true;
			}
		}

	// If we don't already have a message for the caller, generate a "nothing"
	// (this has the lowest priority of all)
	if (!bGotMsgForCaller)
		{
		pmsg->msg.nothing.ucType = NetMsg::NOTHING;
		pmsg->ucSenderID = Net::InvalidID;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Send message to specified client
////////////////////////////////////////////////////////////////////////////////
void CNetServer::SendMsg(
	Net::ID id,												// In:  ID to send message to
	NetMsg* pmsg)											// In:  Message to send
	{
	ASSERT(id != Net::InvalidID);
	ASSERT(id < Net::MaxNumIDs);
	m_aClients[id].m_msgr.SendMsg(pmsg);
	}


////////////////////////////////////////////////////////////////////////////////
// Send message to specified group of clients - only Joined clients are allowed!
////////////////////////////////////////////////////////////////////////////////
void CNetServer::SendMsg(
	U16 mask,												// In:  Bit-mask indicating who to send to
	NetMsg* pmsg)											// In:  Message to send
	{
	// Bit-mask determines who to send the message to
	for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
		{
		if ((mask & 1) && (m_aClients[id].m_state == CClient::Joined))
			SendMsg(id, pmsg);
		mask = mask >> 1;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Send message to all joined clients
////////////////////////////////////////////////////////////////////////////////
void CNetServer::SendMsg(
	NetMsg* pmsg)											// In:  Message to send
	{
	for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
		{
		if (m_aClients[id].m_state == CClient::Joined)
			SendMsg(id, pmsg);
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Determine if there is more data waiting to be sent.  If there is data to
// to be sent AND there is a send error, then that data can't be sent, so we
// return false to indicate "no more data".
////////////////////////////////////////////////////////////////////////////////
bool CNetServer::IsMoreToSend(void)					// Returns true if more to send, false otherwise
	{
	bool bResult = false;
	for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
		{
		if (m_aClients[id].m_msgr.IsMoreToSend())
			{
			bResult = true;
			break;
			}
		}
	return bResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Drop specified client
////////////////////////////////////////////////////////////////////////////////
void CNetServer::DropClient(
	Net::ID id)												// In:  Client to drop
	{
	ASSERT(id != Net::InvalidID);
	ASSERT(id < Net::MaxNumIDs);

	switch(m_aClients[id].m_state)
		{
		case CClient::Unused:
			// This doesn't really make sense because once we're connected we should
			// also be in a state other than "unused".
			ASSERT(0);
			m_aClients[id].m_msgr.Disconnect(false);
			break;

		case CClient::Used:
			m_aClients[id].m_state = CClient::Unused;
			m_aClients[id].m_msgr.Disconnect(true);
			break;

		case CClient::Joined:
			// Whether or not the game has started makes a huge difference in how this is handled
			if (m_bGameStarted)
				{
				// If the queue is empty, we can start the drop process.  Otherwise, we need
				// to add the new drop client to the drop queue.
				if (m_qDropIDs.IsEmpty())
					{
					// Add to queue (we use this as an indicator that we're dropping someone)
					m_qDropIDs.PutAtBack(id);

					// Start the process of dropping a client during a game
					StartDroppingClientDuringGame(id);
					}
				else
					{
					// Add client's ID to drop queue
					m_qDropIDs.PutAtBack(id);
					}
				}
			else
				{
				// Tell all clients to drop this client (including the one being dropped)
				NetMsg msg;
				msg.msg.dropped.ucType = NetMsg::DROPPED;
				msg.msg.dropped.id = id;
				msg.msg.dropped.sContext = -1;
				SendMsg(&msg);

				// Change dropped client's state to "unused" (only AFTER message was sent)
				m_aClients[id].m_state = CClient::Unused;
				}
			m_aClients[id].m_msgr.Disconnect(true);
			break;

		case CClient::Dropped:
			// Nothing to do (game must have already started for it to be in this state)
			break;
		}

	// Clear client's "logged in" flag
	m_aClients[id].m_bLoggedIn = false;
	}


////////////////////////////////////////////////////////////////////////////////
// Send the specified game settings to all joined clients
////////////////////////////////////////////////////////////////////////////////
void CNetServer::SetupGame(
	short sRealmNum,										// In:  Realm number
	const char* pszRealmFile,									// In:  Realm file name
	short sDifficulty,									// In:  Difficulty
	short sRejuvenate,									// In:  Rejuvenate flag
	short sTimeLimit,										// In:  Time limit in minutes, or negative if none
	short sKillLimit,										// In:  Kill limit, or negative if none
	short	sCoopLevels,									// In:  Zero for deathmatch levels, non-zero for cooperative levels.
	short	sCoopMode)										// In:  Zero for deathmatch mode, non-zero for cooperative mode.
	{
	// Setup a special START_GAME message that we keep around so we can send
	// it to clients as soon as they join.  That way, they get better feedback.
	// Otherwise, they would have to wait until the next time this function
	// is called, which may not be all that often.
	m_msgSetupGame.msg.setupGame.ucType	= NetMsg::SETUP_GAME;
	m_msgSetupGame.msg.setupGame.sRealmNum = sRealmNum;
	memcpy(m_msgSetupGame.msg.setupGame.acRealmFile, pszRealmFile, sizeof(m_msgSetupGame.msg.setupGame.acRealmFile));
	m_msgSetupGame.msg.setupGame.acRealmFile[sizeof(m_msgSetupGame.msg.setupGame.acRealmFile)-1] = 0;
	m_msgSetupGame.msg.setupGame.sDifficulty = sDifficulty;
	m_msgSetupGame.msg.setupGame.sRejuvenate = sRejuvenate;
	m_msgSetupGame.msg.setupGame.sTimeLimit = sTimeLimit;
	m_msgSetupGame.msg.setupGame.sKillLimit = sKillLimit;
	m_msgSetupGame.msg.setupGame.sCoopLevels = sCoopLevels;
	m_msgSetupGame.msg.setupGame.sCoopMode = sCoopMode;
	
	// Mark message as valid
	m_bSetupGameValid = true;

	SendMsg(&m_msgSetupGame);
	}


////////////////////////////////////////////////////////////////////////////////
// Start game using specified settings
////////////////////////////////////////////////////////////////////////////////
void CNetServer::StartGame(
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
	Net::SEQ seqMaxAhead)								// In:  Initial max ahead for input versus frame seq
	{
	ASSERT(!m_bGameStarted);

	// Set flag
	m_bGameStarted = true;

	// Tell all joined clients to start game
	NetMsg msg;
	msg.msg.startGame.ucType = NetMsg::START_GAME;
	msg.msg.startGame.idServer = idServer;
	msg.msg.startGame.sRealmNum = sRealmNum;
	memcpy(msg.msg.startGame.acRealmFile, pszRealmFile, sizeof(msg.msg.startGame.acRealmFile));
	msg.msg.startGame.acRealmFile[sizeof(msg.msg.startGame.acRealmFile)-1] = 0;
	msg.msg.startGame.sDifficulty = sDifficulty;
	msg.msg.startGame.sRejuvenate = sRejuvenate;
	msg.msg.startGame.sTimeLimit = sTimeLimit;
	msg.msg.startGame.sKillLimit = sKillLimit;
	msg.msg.startGame.sCoopLevels = sCoopLevels;
	msg.msg.startGame.sCoopMode = sCoopMode;
	msg.msg.startGame.sFrameTime = sFrameTime;
	msg.msg.startGame.seqMaxAhead = seqMaxAhead;
	SendMsg(&msg);

	// We need to wait for clients to respond with realm status messages
	m_bWaitingForRealmStatus = true;
	}


////////////////////////////////////////////////////////////////////////////////
// Abort game
////////////////////////////////////////////////////////////////////////////////
void CNetServer::AbortGame(
	unsigned char ucReason)								// In:  Why game was aborted
	{
	// Tell players to abort game
	NetMsg msg;
	msg.msg.abortGame.ucType = NetMsg::ABORT_GAME;
	msg.msg.abortGame.ucReason = ucReason;
	SendMsg(&msg);
	}


////////////////////////////////////////////////////////////////////////////////
// Tell clients to go to the next realm when they reach the specified frame seq.
// It is okay to call this multiple times -- "extra" calls are ignored.
//
// The return value is true if a new "next realm" sequence was initiated, and
// false if there was already one in progress.
////////////////////////////////////////////////////////////////////////////////
bool CNetServer::NextRealm(
	Net::SEQ seq)											// In:  The seq on which to go to next realm
	{
	// If we're already waiting, then we don't send this message.  There are
	// two situations this can come up.  First, the caller could have already
	// called us once, and is merely calling us over and over, which, as the
	// the function header comment says, is allowed.  The second possibility
	// is that not all clients have responded to a much older attempt to
	// start the game or go to the next realm, and now the caller wants to go
	// on to yet another realm.  In that situation, we can't accomodate the
	// caller -- we have to wait for all the clients to catch up to the previous
	// realm before we can go to the next one.  All in all, this is rather
	// involved explanation for what works out to a simple check of a flag...
	bool bResult = true;
	if (!m_bWaitingForRealmStatus)
		{
		// Tell clients to go to the next realm when they reach this frame seq
		NetMsg msg;
		msg.msg.nextRealm.ucType = NetMsg::NEXT_REALM;
		msg.msg.nextRealm.seqHalt = seq;
		SendMsg(&msg);

		// We need to wait for clients to respond with realm status messages
		m_bWaitingForRealmStatus = true;
		}
	else
		{
		// There was already a "next realm" sequence in effect
		bResult = false;
		}
	return bResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Tell clients to proceed.  This is used when all players are at a point where
// they need to be told it's time to move on.  There is NO attempt to
// syncrhonize the clients, so this must only be used in non-critical sections
// or at a point prior to a syncrhonized section.  For instance, if all players
// are viewing a dialog, the local user on the server might hit a key to go
// on, at which point this function would be called to tell all the other
// players to proceed.
////////////////////////////////////////////////////////////////////////////////
void CNetServer::Proceed(void)
	{
	// Send PROCEED message to all joined clients
	NetMsg msg;
	msg.msg.proceed.ucType = NetMsg::PROCEED;
	SendMsg(&msg);
	}


////////////////////////////////////////////////////////////////////////////////
// Start the process of dropping a client during a game
////////////////////////////////////////////////////////////////////////////////
void CNetServer::StartDroppingClientDuringGame(
	Net::ID id)
	{
	// Tell all clients to drop this client (including the one being dropped)
	// Because there's something in the drop queue, GetMsg() will know that
	// it should be exptecting DROP_ACK messages from the remaining clients.
	NetMsg msg;
	msg.msg.dropped.ucType = NetMsg::DROPPED;
	msg.msg.dropped.id = id;
	msg.msg.dropped.sContext = 1;
	SendMsg(&msg);

	// Change dropped client's state to "dropped" (only AFTER message was sent)
	m_aClients[id].m_state = CClient::Dropped;

	// Clear "sent drop ack" flags for all clients
	for (Net::ID id2 = 0; id2 < Net::MaxNumIDs; id2++)
		m_aClients[id2].m_bSentDropAck = false;
	}


////////////////////////////////////////////////////////////////////////////////
// Got all drop acks, now figure out what to do next
//
// A return value of true indicates that one or more clients needed inputs,
// so the drop process is not yet complete.  A return of false indicates that
// no clients needed inputs, so we can proceed directly to the next phase.
////////////////////////////////////////////////////////////////////////////////
bool CNetServer::GotAllDropAcks(void)
	{
	// Go through all the responses and find
	//		(1) the lowest frame seq,
	//		(2) the highest frame seq
	//		(3) the highest known dropee input seq
	// To do this, we need to start out with some valid values, which we
	// get from the first client we come across.  We can't do a typical thing
	// where you set the values to a very high or very low value to initial
	// it, because when comparing seq's, it is very important that they be
	// withing a small range of one another, or all bets are off.
	bool bFirst = true;
	m_seqHighestDoneFrame = 0;
	Net::ID idHighestDoneFrame = Net::InvalidID;
	Net::SEQ seqHighestDropeeInput = 0;					// Used purely for debugging/verification
	Net::SEQ seqLowestDropeeInput = 0;
	Net::ID id;
	for (id = 0; id < Net::MaxNumIDs; id++)
		{
		if (m_aClients[id].m_state == CClient::Joined)
			{
			if (bFirst)
				{
				// Fill in the values from the first client we get to
				m_seqHighestDoneFrame	= m_aClients[id].m_seqLastDoneFrame;
				idHighestDoneFrame		= id;
				seqHighestDropeeInput	= m_aClients[id].m_seqHighestDropeeInput;
				seqLowestDropeeInput		= m_aClients[id].m_seqHighestDropeeInput;
				bFirst = false;
				}
			else
				{
				// If client's frame is >= highest frame, then this is the new highest frame
				if (SEQ_GTE(m_aClients[id].m_seqLastDoneFrame, m_seqHighestDoneFrame))
					{
					m_seqHighestDoneFrame = m_aClients[id].m_seqLastDoneFrame;
					idHighestDoneFrame = id;
					}

				// If client's input is >= highest input, then this is the new highest input
				if (SEQ_GTE(m_aClients[id].m_seqHighestDropeeInput, seqHighestDropeeInput))
					{
					seqHighestDropeeInput = m_aClients[id].m_seqHighestDropeeInput;
					}

				// If client's input is <= lowest input, then this is the new lowest input
				if (SEQ_LTE(m_aClients[id].m_seqHighestDropeeInput, seqLowestDropeeInput))
					{
					seqLowestDropeeInput = m_aClients[id].m_seqHighestDropeeInput;
					}
				}
			}
		}

	// If we don't come out of this with a valid ID, we're in deep shit
	ASSERT(idHighestDoneFrame != Net::InvalidID);

	// Make sure someone has the dropee's input up to and including the highest frame.
	// This should always be the case because if someone did a frame, they must have
	// had the dropee's input for that frame.
	ASSERT(SEQ_GTE(seqHighestDropeeInput, m_seqHighestDoneFrame));

	// Go through all the clients and see if any one of them needs inputs
	bool bSomeoneNeedsInputs = false;
	for (id = 0; id < Net::MaxNumIDs; id++)
		{
		if (m_aClients[id].m_state == CClient::Joined)
			{
			// If this client's highest dropee input is <= the highest frame, then he needs inputs
			if (SEQ_LT(m_aClients[id].m_seqHighestDropeeInput, m_seqHighestDoneFrame))
				{
				bSomeoneNeedsInputs = true;
				break;
				}
			}
		}

	// If someone needs inputs, get them from whoever had the highest done frame
	if (bSomeoneNeedsInputs)
		{
		// We need the inputs starting at the lowest dropee input + 1 (they already
		// have the lowest one) up to the highest done frame.
		NetMsg msg;
		msg.msg.inputReq.ucType = NetMsg::INPUT_REQ;
		msg.msg.inputReq.id = m_qDropIDs.PeekAtFront();
		msg.msg.inputReq.seqStart = (Net::SEQ)(seqLowestDropeeInput + (Net::SEQ)1);
		msg.msg.inputReq.sNum = (Net::SEQ)(m_seqHighestDoneFrame - seqLowestDropeeInput);
		SendMsg(idHighestDoneFrame, &msg);

		// Set flag to indicate that we're waiting for inputs
		m_bWaitingForInputData = true;
		}

	return bSomeoneNeedsInputs;
	}


////////////////////////////////////////////////////////////////////////////////
// Finish the process of dropping client.  If there's more clients in the drop
// queue, this could also start the dropping of the next victim
////////////////////////////////////////////////////////////////////////////////
void CNetServer::FinishDroppingClientDuringGame(void)
	{
	// Send INPUT_MARK message to all clients so they can mark the last active
	// input seq of the dropped client.
	NetMsg msg;
	msg.msg.inputMark.ucType = NetMsg::INPUT_MARK;
	msg.msg.inputMark.id = m_qDropIDs.PeekAtFront();
	msg.msg.inputMark.seqMark = m_seqHighestDoneFrame;
	SendMsg(&msg);

	// This client is now fully dropped, so remove it from the drop queue
	m_qDropIDs.GetFromFront();

	// Check if there's any more clients in the queue.  If not, everyone can
	// resume playing.  Otherwise, drop the next client in the queue.
	if (m_qDropIDs.IsEmpty())
		{
		// Send START_REALM message to all joined clients
		msg.msg.startRealm.ucType = NetMsg::START_REALM;
		SendMsg(&msg);

		// Reset all related flags
		m_bWaitingForRealmStatus = false;
		for (Net::ID id2 = 0; id2 < Net::MaxNumIDs; id2++)
			m_aClients[id2].m_bSentReadyRealm = false;
		}
	else
		{
		// Start the process of dropping the next client in the drop queue.
		StartDroppingClientDuringGame(m_qDropIDs.PeekAtFront());
		}
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
