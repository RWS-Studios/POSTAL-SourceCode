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
// socket.h
// Project: Postal
// 
// History:
//		02/19/97 MJR	Started.
//
//		04/08/97 MJR	Added class and function names to all TRACE() messages.
//
//							Added test for minimum number of sockets supported.
//
//							Fixed problem that would have occurred if an error occurred
//							when removing the blocking hook.
//
//		04/12/97 MJR	Distilled this file out of what was gamelink.cpp in order
//							to separate the socket stuff from the game stuff.
//
//							Major changes to create a simplified, generic,
//							socket-oriented interface.
//
//		04/13/97 MJR	Continued a huge number of changes, including many of
//							the outstanding issues that had been listed in the
//							comment header of the previous incarnation of this file.
//
//		04/14/97 MJR	Lots of testing, debugging, fixing, etc.
//
//		05/20/97 MJR	Minor changes.
//
//		05/21/97 MJR	Pulled "listen" option out of Open() and made it a
//							separate function.
//
//							Created two Open() variations, one for datagrams and one
//							for streams, each with separate options.
//
//		05/23/97 MJR	Renamed parameters to GetAddress().
//
//		05/25/97 MJR	Changed type in Address from int (iLen) to long (lLen).
//
//		05/25/97 MJR	Added ability to set callback via Open().
//
//		05/26/97 MJR	Changed IsError() so it no longer considers a closed
//							socket to be an error.
//
//		08/03/97 BRH	Added IPX network support.
//
//		08/04/97 BRH	Restarted this file to use the new plugin protocols
//
//		08/08/97 MJR	Another round of cleanups, and added support for
//							handling Win32 -vs- Mac protocols.
//
//		08/09/97 MJR	Added broadcast support.
//							Added check to be sure all addresses are the same size.
//
//		08/12/97 MJR	Added mac protocols.
//
//		08/18/97 MJR	Now trims leading and trailing whitespace from names.
//					MJR	Tweaked for mac.
//
//		08/20/97 MJR	Added support for setting whether socket blocks or not.
//
//		11/19/97	JMI	Added "MPATH" string for corresponding MPATH enum in
//							socket.h.  Since there was no string for this protocol,
//							when toggling through the various protocols in the 
//							multiplayer options menu, you would get random varing 
//							strings for the 3rd protocol (MPATH).
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>

#include "RSPiX.h"
#include "socket.h"


////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// RSocket static member variables
////////////////////////////////////////////////////////////////////////////////

bool							RSocket::ms_bDidStartup = false;
bool							RSocket::ms_bAutoShutdown = false;
RSocket::ProtoType		RSocket::ms_prototype   = RSocket::NO_PROTOCOL;
short							RSocket::ms_sNumSockets = 0;
char*							RSocket::ms_apszProtoNames[] = 
									{
									"",
//									"Loopback",

										"TCP/IP",
									};


////////////////////////////////////////////////////////////////////////////////
// RProtocol static member variables
////////////////////////////////////////////////////////////////////////////////

RSocket::ProtoType		RSocket::RProtocol::ms_prototype = RSocket::NO_PROTOCOL;


////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////
RSocket::RSocket()
	{
	m_pProtocol = 0;

	// Update number of sockets
	ms_sNumSockets++;
	}


////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
RSocket::~RSocket()
	{
	Reset();
	
	// Update number of sockets
	ms_sNumSockets--;
	
	// If there's no more sockets and auto-shutdown is enabled, then shtudown
	if ((ms_sNumSockets == 0) && ms_bAutoShutdown)
		Shutdown();
	}


////////////////////////////////////////////////////////////////////////////////
// Reset socket to its post-construction state
////////////////////////////////////////////////////////////////////////////////
void RSocket::Reset(void)
	{
	// Force it to close now
	Close(true);

	// If close failed, we're going to delete protocol anyway as part of reset
	delete m_pProtocol;
	
	m_pProtocol = NULL;
	}


////////////////////////////////////////////////////////////////////////////////
// Open socket
//
// If the current protocol is not supported, this function returns the value
// RSocket::errNotSupported.
////////////////////////////////////////////////////////////////////////////////
short RSocket::Open(										// Returns 0 on success, non-zero otherwise
	unsigned short usPort,								// In:  Port number or 0 for any port
	short sType,											// In:  Any one RSocket::typ* enum
	short sOptionFlags,									// In:  Any combo of RSocket::opt* enums
	RSocket::BLOCK_CALLBACK callback)				// In:  Blocking callback (or NULL to keep current)
	{
	// Make sure startup was called.  Only this function needs to do this
	// because all the others check for a valid protocol, which can only be
	// created via this function.
	short sResult = 0;
	
	if (ms_bDidStartup)
		{
		// Make sure socket isn't already open
		if (m_pProtocol == NULL)
			{
			// Note that we create the protocol object here instead of when this
			// socket was itself constructed.  This is preferable because users might
			// want to create a static RSocket object, which would occur before the
			// protocol has been set via RSocket::Startup, and therefore wouldn't be
			// able to know what type of protocol object to create.  By deferring the
			// creation of the protocol to this point, we allow for the use of static
			// RSocket objects.
			m_pProtocol = ConstructProtocol(ms_prototype);
			if (m_pProtocol != NULL)
				{
				sResult = m_pProtocol->Open(usPort, sType, sOptionFlags, callback);
				}
			else
				{
				sResult = -1;
				TRACE("RSocket::Open(): Error constructing protocol!\n");
				}
			
			// If there was a problem, get rid of the protocol
			if (sResult != 0)
				{
				delete m_pProtocol;
				m_pProtocol = 0;
				}
			}
		else
			{
			sResult = -1;
			TRACE("RSocket::Open(): Already open!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RSocket::Open(): Didn't call RSocket::Startup()!\n");
		}
	
	return sResult; 
	}


////////////////////////////////////////////////////////////////////////////////
// Close socket
////////////////////////////////////////////////////////////////////////////////
short RSocket::Close(									// Returns 0 if successfull, non-zero otherwise
	bool bForceNow /*= true */)						// In:  'true' means do it now, false follows normal rules
	{
	short sResult = 0;

	if (m_pProtocol != NULL)
		{
		// Close socket
		sResult = m_pProtocol->Close(bForceNow);
		if (sResult == 0)
			{
			// Get rid of protocol
			delete m_pProtocol;
			m_pProtocol = NULL;
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Accept request for connection.
//
// If this function fails, the specified client socket and address may have
// been modified, but any such changes must not be relied upon!!!  What can be
// relied upon is that the client socket will be in a "closed" state.
////////////////////////////////////////////////////////////////////////////////
short RSocket::Accept(									// Return 0 if successfull, non-zero otherwise
	RSocket* psocketClient,								// Out: Client socket returned here
	RSocket::Address* paddressClient) const		// Out: Client's address returned here 
	{
	short sResult = FAILURE;
	
	if (m_pProtocol != NULL)
		{
		// Make sure client doesn't already have a protocol
		if (psocketClient->m_pProtocol == 0)
			{
			// Create protocol
			psocketClient->m_pProtocol = ConstructProtocol(ms_prototype);
			if (psocketClient->m_pProtocol != NULL)
				{
				sResult = m_pProtocol->Accept(psocketClient->m_pProtocol, paddressClient);

				// If there was a problem, get rid of the protocol
				if (sResult != 0)
					{
						// Get rid of protocol
						delete psocketClient->m_pProtocol;
						psocketClient->m_pProtocol = 0;
					}
				}
			else
				{
				sResult = -1;
				TRACE("RSocket::Accept(): Couldn't construct client protocol!\n");
				}
			}
		else
			{
				sResult = -1;
				TRACE("RSocket::Accept(): Client socket already has a protocol!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RSocket::Accept(): Not open!\n");
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Set socket to broadcast mode
//
// Most protocols only allow broadcasting on a datagram-style socket.
////////////////////////////////////////////////////////////////////////////////
short RSocket::Broadcast(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = FAILURE;
	
	if (m_pProtocol != NULL)
		sResult = m_pProtocol->Broadcast();	
	else
		{
		sResult = -1;
		TRACE("RSocket::Broadcast(): Not open!\n");
		}
	
	return sResult;	
	}


////////////////////////////////////////////////////////////////////////////////
// Set socket to listen for connection requests
////////////////////////////////////////////////////////////////////////////////
short RSocket::Listen(short sMaxQueued)
	{
	short sResult = FAILURE;
	
	if (m_pProtocol != NULL)
		sResult = m_pProtocol->Listen(sMaxQueued);	
	else
		{
		sResult = -1;
		TRACE("RSocket::Listen(): Not open!\n");
		}
	
	return sResult;	
	}


////////////////////////////////////////////////////////////////////////////////
// Connect to address
////////////////////////////////////////////////////////////////////////////////
short RSocket::Connect(
	RSocket::Address* paddress)
	{
	short sResult = 0;
	
	if (m_pProtocol != NULL)
		sResult = m_pProtocol->Connect(paddress);
	else
		{
		sResult = -1;
		TRACE("RSocket::Connect(): Not open!\n");
		}
	
	return sResult;
	} 


////////////////////////////////////////////////////////////////////////////////
// Use Send for connected sockets - use SendTo for connectionless
////////////////////////////////////////////////////////////////////////////////
short RSocket::Send(										// Return 0 on success, non-zero otherwise
	void* pBuf,												// In:  Pointer to data buffer
	long lNumBytes,										// In:  Number of bytes to send
	long* plActualBytes)									// Out: Actual number of bytes sent
	{
	short sResult = FAILURE;
	
	if (m_pProtocol != NULL)
		sResult = m_pProtocol->Send(pBuf, lNumBytes, plActualBytes);
	else
		{
		sResult = -1;
		TRACE("RSocket::Send(): Not open!\n");
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Send data to specified address.  For connected sockets, address is ignored
// See Send() for more information.
////////////////////////////////////////////////////////////////////////////////
short RSocket::SendTo(									// Return 0 on success, non-zero otherwise
	void* pBuf,												// In:  Pointer to data buffer
	long lNumBytes,										// In:  Number of bytes to send
	long* plActualBytes,									// Out: Actual number of bytes sent
	RSocket::Address* paddress)						//In:  Address to send to
	{
	short sResult = FAILURE;
	
	if (m_pProtocol != NULL)
		sResult = m_pProtocol->SendTo(pBuf, lNumBytes, plActualBytes, paddress);
	else
		{
		sResult = -1;
		TRACE("RSocket::SendTo(): Not open!\n");
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Receive data (only valid for connected sockets -- see also ReceiveFrom())
//
// For datagram (UDP) sockets, there is a one-to-one correspondence between
// sends and receives.  Each send implies a matching receive.  The specified
// buffer must at least as large as the amount of data that was sent, or the
// data will be truncated and an error will be returned.
//
// For stream (TCP) sockets, there is no direct correspondence between sends
// and receive.  One send can be broken up and require multiple receives, and
// and multiple sends can be coalesced into a single recieve.  There is no
// limitation on the amount of data being received.
//
// For stream (TCP) sockets, if the actual number of bytes received is 0, it
// means the other end disconnected gracefully.
//
// In all cases, if the connection was abortively disconnected, an error will
// be returned.
////////////////////////////////////////////////////////////////////////////////
short RSocket::Receive(									// Returns 0 on success, non-zero otherwise
	void* pBuf,												// In:  Pointer to data buffer
	long lMaxBytes,										// In:  Maximum bytes that can fit in buffer
	long* plActualBytes)									// Out: Actual number of bytes received
	{
	short sResult = FAILURE;
	
	if (m_pProtocol != NULL)
		sResult = m_pProtocol->Receive(pBuf, lMaxBytes, plActualBytes);
	else
		{
		sResult = -1;
		TRACE("RSocket::Receive(): Not open!\n");
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Receive data and get source address
////////////////////////////////////////////////////////////////////////////////
short RSocket::ReceiveFrom(							// Returns 0 on success, non-zero otherwise
	void* pBuf,												// In:  Pointer to data buffer
	long lMaxBytes,										// In:  Maxiumm bytes that fit in buffer
	long* plActualBytes,									// Out: Actual number of bytes received into buffer
	RSocket::Address* paddress)						// Out: Source address returned here
	{
	short sResult = FAILURE;
	
	if (m_pProtocol != NULL)
		sResult = m_pProtocol->ReceiveFrom(pBuf, lMaxBytes, plActualBytes, paddress);
	else
		{
		sResult = -1;
		TRACE("RSocket::ReceiveFrom(): Not open!\n");
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Check the number of bytes of data receivable without blocking.
// For datagrams, this returns the size of the next queued datagram.  For
// streams, this returns the total amount of data that can be read with a
// single receive which is normally equal to the total amount of queued data.
////////////////////////////////////////////////////////////////////////////////
long RSocket::CheckReceivableBytes(void)
	{
	long lResult = 0;
	
	if (m_pProtocol != NULL)
		lResult = m_pProtocol->CheckReceivableBytes();
	
	return lResult;
	}


////////////////////////////////////////////////////////////////////////////////
// CanAcceptWithoutBlocking
////////////////////////////////////////////////////////////////////////////////
bool RSocket::CanAcceptWithoutBlocking(void)
	{
	bool bNoBlock = false;
	
	if (m_pProtocol != NULL)
		bNoBlock = m_pProtocol->CanAcceptWithoutBlocking();
	
	return bNoBlock;
	}


////////////////////////////////////////////////////////////////////////////////
// CanSendWithoutBlocking
////////////////////////////////////////////////////////////////////////////////
bool RSocket::CanSendWithoutBlocking(void)
	{
	bool bNoBlock = false;
	
	if (m_pProtocol != NULL)
		bNoBlock = m_pProtocol->CanSendWithoutBlocking();
	
	return bNoBlock;
	}


////////////////////////////////////////////////////////////////////////////////
// CanReceiveWithoutBlocking
////////////////////////////////////////////////////////////////////////////////
bool RSocket::CanReceiveWithoutBlocking(void)
	{
	bool bNoBlock = false;
	
	if (m_pProtocol != NULL)
		bNoBlock = m_pProtocol->CanReceiveWithoutBlocking();
	
	return bNoBlock;
	}


////////////////////////////////////////////////////////////////////////////////
// Check socket's error status.  If the socket is not open, then the return 
// value will be false (no error)
////////////////////////////////////////////////////////////////////////////////
bool RSocket::IsError(void)
	{
	bool bResult = false;
	
	if (m_pProtocol != NULL)
		bResult = m_pProtocol->IsError();
	
	return bResult;
	}


////////////////////////////////////////////////////////////////////////////////
// The specified protocol becomes the current protocol for all RSocket objects.
// The protocol can NOT be changed while any sockets exist!!!  In other words,
// all RSocket objects must be destroyed before the protocol can be changed.
//
// Specifiying true for bAutoShutdown means that Shutdown() will be called
// when the last RSocket object is destroyed.  This is generally acceptable,
// but could potentially cause problems if Startup() and Shutdown() are lengthy
// processes, which is entirely protocol-dependant.
//
////////////////////////////////////////////////////////////////////////////////
// static
short RSocket::Startup(						// Returns 0 if successfull, non-zero otherwise
	RSocket::ProtoType prototype,			// In:  Protocol type
	bool bAutoShutdown)						// In:  Whether to perform auto Shutdown()
	{
	short sResult = 0;
	
	// Only do this once
	if (!ms_bDidStartup)
		{
		// Make sure no sockets exist
		if (ms_sNumSockets == 0)
			{
			// Set new stuff
			ms_prototype = prototype;
			ms_bAutoShutdown = bAutoShutdown;

			// Call the protocol's startup
			switch (ms_prototype)
				{
				case RSocket::TCPIP:
					ASSERT(sizeof(RProtocolBSDIP::AddressIP) == sizeof(RSocket::Address));
					sResult = RProtocolBSDIP::Startup();
					break;

				default:
					sResult = -1;
					TRACE("RSocket::Startup(): Unknown procol!\n");
					break;
				}
			}
		else
			{
			sResult = -1;
			TRACE("RSocket::Startup(): Can't change protocol -- %hd RSockets still exist!\n", (short)ms_sNumSockets);
			}

		if (sResult == 0)
			ms_bDidStartup = true;
		}
		
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown
////////////////////////////////////////////////////////////////////////////////
// static
void RSocket::Shutdown(void)
	{
	switch (ms_prototype)
		{
		case RSocket::TCPIP:
			RProtocolBSDIP::Shutdown();
			break;

		default:
			TRACE("RSocket::Shutdown(): Unknown protocol!\n");
			break;
		}
	ms_bDidStartup = false;
	
	// Clear protocol so we know it's safe to change it
	ms_prototype = RSocket::NO_PROTOCOL;
	}


////////////////////////////////////////////////////////////////////////////////
// Get maximum datagram size.  A value of 0 indicates that there is no 
// limitation on size.
//
// NOTE: This uses the protocol selected via RSocket::Startup().  If this
// function is called before any protocol has been selected, it fails.
////////////////////////////////////////////////////////////////////////////////
// static
short RSocket::GetMaxDatagramSize(					// Returns 0 on success, non-zero otherwise
	long* plSize)											// Out: Maximum datagram size (in bytes)
	{
	short sResult = FAILURE;
	
	switch (ms_prototype)
		{
		case RSocket::TCPIP:
			sResult = RProtocolBSDIP::GetMaxDatagramSize(plSize);
			break;

		default:
			sResult = -1;
			TRACE("RSocket::GetMaxDatagramSize(): Unknown protocol!\n");
			break;
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Get maximum number of sockets.  This may be a system "global" value, which
// means that if other applications are using sockets, then the number available
// to this application may be lower than the returned value.
//
// NOTE: This uses the protocol selected via RSocket::Startup().  If this
// function is called before any protocol has been selected, it fails.
////////////////////////////////////////////////////////////////////////////////
// static
short RSocket::GetMaxSockets(							// Returns 0 on success, non-zero otherwise
	long* plNum)											// Out: Maximum number of sockets
	{
	short sResult = FAILURE;
	
	switch (ms_prototype)
		{
		case RSocket::TCPIP:
			sResult = RProtocolBSDIP::GetMaxSockets(plNum);
			break;

		default:
			sResult = -1;
			TRACE("RSocket::GetMaxSockets(): Unknown protocol!\n");
			break;
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Get address of specified host
//
// NOTE: This uses the protocol selected via RSocket::Startup().  If this
// function is called before any protocol has been selected, it fails.
////////////////////////////////////////////////////////////////////////////////
// static
short RSocket::GetAddress(								// Returns 0 on success, non-zero otherwise
	char* pszName,											// In:  Host's name or dotted addres (x.x.x.x)
	USHORT usPort,											// In:  Host's port number
	RSocket::Address* paddress)						// Out: Address
	{
	short sResult = 0;
	
	// Get rid of leading and trailing whitespace
	char azName[RSP_MAX_PATH];
	if (strlen(pszName) < RSP_MAX_PATH)
		{
		// Skip over leading whitespace
		while ((*pszName != 0) && isspace(*pszName))
			pszName++;

		// Copy resulting string
		strcpy(azName, pszName);

		// Convert trailing whitespace to 0's
		long index;
		for (index = strlen(azName) - 1; index >= 0; index--)
			{
			if (isspace(azName[index]))
				azName[index] = 0;
			else
				break;
			}
		if (index < 0)
			{
			sResult = -1;
			TRACE("RSocket::GetAddress(): String is empty (after removing leading & trailing space)\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RSocket::GetAddress(): Specified string is too long!\n");
		}

	// Make sure we don't accidently use this pointer -- must use azName instead!
	pszName = 0;

	// If the name is ready, try to get the address
	if (sResult == 0)
		{
		switch (ms_prototype)
			{
			case RSocket::TCPIP:
				sResult = RProtocolBSDIP::GetAddress(azName, usPort, paddress);
				break;

			default:
				sResult = -1;
				TRACE("RSocket::GetAddress(): Unknown protocol!\n");
				break;
			}
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Create broadcast address using specified port
////////////////////////////////////////////////////////////////////////////////
// static
void RSocket::CreateBroadcastAddress(
	unsigned short usPort,								// In:  Port to broadcast to
	RSocket::Address* paddress)						// Out: Broadcast address returned here
	{
	switch (ms_prototype)
		{
		case RSocket::TCPIP:
			RProtocolBSDIP::CreateBroadcastAddress(usPort, paddress);
			break;

		default:
			TRACE("RSocket::GetAddress(): Unknown protocol!\n");
			break;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Get the port of an existing (valid) address
//
// NOTE: This uses the protocol selected via RSocket::Startup().  If this
// function is called before any protocol has been selected, it fails.
////////////////////////////////////////////////////////////////////////////////
// static
unsigned short RSocket::GetAddressPort(			// Returns the port number
	RSocket::Address* paddress)						// In:  Address to get port from
	{
	unsigned short usPort = 0;
	
	switch (ms_prototype)
		{
		case RSocket::TCPIP:
			usPort = RProtocolBSDIP::GetAddressPort(paddress);
			break;

		default:
			TRACE("RSocket::GetAddressPort(): Unknown protocol!\n");
			break;
		}
	
	return usPort;
	}


////////////////////////////////////////////////////////////////////////////////
// Set the port of an existing (valid) address
//
// NOTE: This uses the protocol selected via RSocket::Startup().  If this
// function is called before any protocol has been selected, it fails.
////////////////////////////////////////////////////////////////////////////////
// static
void RSocket::SetAddressPort(
	USHORT usPort,											// In:  New port number
	RSocket::Address* paddress)						// I/O: Address whose port is to be set
	{
	switch (ms_prototype)
		{
		case RSocket::TCPIP:
			RProtocolBSDIP::SetAddressPort(usPort, paddress);
			break;

		default:
			TRACE("RSocket::SetAddressPort(): Unknown protocol!\n");
			break;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Create specified protocol object
////////////////////////////////////////////////////////////////////////////////

// since there can only be one modem in use (unless the user is stupid and wears
// false antlers), we only need one instance on RProtocolTAPI, which is global:
RSocket::RProtocol* RSocket::ConstructProtocol(	// Returns pointer to prototype if successfull, 0 otherwise
	RSocket::ProtoType prototype)						// In:  Protocol type to create
	{
	RProtocol* pprotocol = 0;
	
	switch (prototype)
		{
		case RSocket::TCPIP:
			pprotocol = new RProtocolBSDIP;
			break;

		default:
			TRACE("RSocket::ConstructProtocol(): Unknown protocol!\n");
			break;
		}

	return pprotocol;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
