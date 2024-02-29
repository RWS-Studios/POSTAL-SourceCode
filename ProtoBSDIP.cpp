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
// ProtoBSDIP.cpp
// Project: Postal
// 
// History:
//		08/04/97 BRH	Started.
//
//		08/06/97 BRH	Debugged - restored to working order.
//					MJR
//
//		08/07/97 BRH	Fixed error message that referred to RSocket
//
//		08/08/97 MJR	Upgraded to latest revision of plugin stuff.
//
//		08/09/97 MJR	Added broadcast support.
//
//		08/14/97 MJR	Fixed GetAddress() to avoid database function when
//							working with a dotted address.
//
//		08/18/97 MJR	Fixed stupid error in GetAddress() that caused crashes.
//
//		08/20/97 MJR	Added support for setting whether socket blocks or not.
//
//		09/02/97 MJR	Got rid of TRACE() on "would block" situations.
//
//					MJR	Fixed serious bug that caused functions to return -1
//							as the number of actual bytes sent/received if an
//							error occurred -- they now properly return 0 instead.
//
//		09/09/97 MJR	Added RSocket::errNotSupported return value to Open().
//
//		11/18/97	JMI	Added message and Close() in destructor for case when
//							an opened socket is never closed.  Trying to find memory
//							leak but this was apparently not the problem.
//
//////////////////////////////////////////////////////////////////////////////
//
// WINSOCK NOTES
// -------------
//
//
// Windows NT WorkStation 4.0 only allow a maximum of 5 backlogs on listen().
// Earlier versions of NT were also limited to 5.  NT servers defaults to 200.
// No idea what Win95 does.
//
// Sockets allow for "stream" (TCP) and "datagram" (UDP).  TCP is referred to
// as "connected", while UDP is normally connectionless.
//
// You can use UDP in connected mode, which saves you from having to specify
// the destination for each send operation and also automatically discards any
// data received from anywhere other than that same destination.  This may be
// faster, depending on the WinSock implimentation.
//
// I suppose one important advantage of being "unconnected" is that you can
// send and receive to/from multiple remote systems, which may be more suitable
// if you were doing some sort of "star" network, where any system can talk to
// any other system.  It seems, though, that you could simulate this with TCP
// or connected UDP by using multiple ports, one per remote system.
//
// TCP is reliable, UDP is not.
//
// With TCP, data may be "coalesced" into larger packets before transmission,
// which is why you need to keep calling receive until you get the amount of
// data you were expecting.  (I also read that you might have to send until
// all the data has been sent, which may make some sense -- you send 100 bytes,
// but the system only needed 50 to "complete" the current data buffer, and so
// it sends only 50 of the 100 you gave it and returns, so you have to call it
// again to send the remaining 50).
//
// With UDP, data is NEVER coalesced, as it may be with TCP.  In both cases,
// data may be split up into multiple blocks based on the maximum transmission
// unit (MTU).  This is supposed to be transparent to the app, and it's obvious
// how it would be with TCP, but I don't see how it could be with UDP since
// it's supposedly unreliable.
//
// We can check the MTU at some point when the socket stuff hands back info
// about the selected protocol (can't remember exact spot, but it exists).
// Other documentation claims that there is no defined way to determine the
// MTU.
//
// We might want to consider the user of broadcasting on LANs.  We might also
// consider using UDP on LAN's (doing our own sequencing and resending of data)
// and TCP on WAN's.  This may work well for another reason, which is that
// I've read several times that TCP headers are usually compressed over PPP
// links, which most people use to connect to WAN's.
//
// PUSH Bit Interpretation
// By default, Windows NT versions 4.0 and 3.5x complete a recv() call when:
// ·	Data arrives with the PUSH bit set.
// ·	The user recv() buffer is full.
// ·	0.5 seconds have elapsed since any data arrived.
// If a client program is run on a computer with a TCP/IP implementation that
// does not set the PUSH bit on sends, response delays may result.  It’s best
//  to correct this on the client side; however, a configuration parameter
// (IgnorePushBitOnReceives) is added to Afd.sys to force it to treat all
// arriving packets as though the PUSH bit were set. 
//
// Delayed Acknowledgments
// Per RFC 1122, TCP uses delayed acknowledgments to reduce the number of
// packets sent on the media. The Microsoft stack takes a common approach
// to implementing delayed acknowledgments. The following conditions cause
// an acknowledgment to be sent as data is received by TCP on a given
// connection:
// ·	No ACK is sent for the previous received segment.
// ·	Segment is received, and no other segment arrives within 200ms for that connection.
// In summary, normally an ACK is sent for every other TCP segment received
// on a connection, unless the delayed ACK timer (200ms) expires. There is no
// configuration parameter to disable delayed ACKs.
//
//
// THINGS TO CHANGE
// ----------------
//
// Do we need to impliment a checksum on UDP data?
//
// We need to check what type of command is being cancelled via the blocking
// hook.  If it's an acceptable command, there's no problem.  If not, then
// the socket becomes unusable.  It would be good if this could be done in some
// way as to be transparent to the app.  Then again, that might not make sense,
// because if certain operations are cancelled, it might make sense that the
// socket is unusable.  I'm no longer sure this is really necessary.  I think
// the general implication should be that any time a function fails, you can
// no longer use that socket, whether it failed because it was aborted or for
// some other reason.
//
// Check into how connections are currently being terminated.  Simply calling
// closesocket() sounds like it would work, but I'm not sure how the other end
// will react.  This needs to be worked out and implimented.
//
// Should we allow user to specify send and recieve buffer sizes, which some
// winsock implimentations support?
//
//////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "socket.h"


//////////////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Static data
//////////////////////////////////////////////////////////////////////////////

WSADATA							RProtocolBSDIP::ms_WSAData;
bool								RProtocolBSDIP::ms_bDidStartup;
bool								RProtocolBSDIP::ms_bWSAStartup;
#ifdef WIN32
bool								RProtocolBSDIP::ms_bWSASetBlockingHook;
RSocket::FuncNum				RProtocolBSDIP::ms_funcnum;
RSocket::BLOCK_CALLBACK		RProtocolBSDIP::ms_callback;
#endif

//////////////////////////////////////////////////////////////////////////////
// Functions
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
RProtocolBSDIP::RProtocolBSDIP(void)
	{
	Init();
	}


//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
RProtocolBSDIP::~RProtocolBSDIP(void)
	{
	if (m_sock != INVALID_SOCKET)
		{
		TRACE("~RProtocolBSDIP(): Closing socket you forgot to, hoser!\n");

		Close();
		}
	}


//////////////////////////////////////////////////////////////////////////////
// Reset
//////////////////////////////////////////////////////////////////////////////
void RProtocolBSDIP::Reset(void)
	{
	Close();
	Init();
	RProtocol::Reset();
	}


//////////////////////////////////////////////////////////////////////////////
// Init
//////////////////////////////////////////////////////////////////////////////
void RProtocolBSDIP::Init(void)
	{
	m_sock = INVALID_SOCKET;
	RProtocol::Init();
	}


//////////////////////////////////////////////////////////////////////////////
//	Startup socket API.  This is normally called from the socket's Startup
// function.  Since we have multiple protocols using the same winsock
// code, the Startup and Shutdown will have to be called after creating
//	an RSocket and before calling Open()
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::Startup(void)
	{
	short sResult = 0;
	
	// Only do this once, but multiple calls are not considered an error
	if (!ms_bDidStartup)
		{
		// Startup winsock API.  If this fails, the typical call the WSGetLastError() is
		// not supported, so there's no way to get a specific error code.
		if (WSAStartup(MAKEWORD(1,1), &ms_WSAData) == 0)
			{
			ms_bWSAStartup = true;
			
			#ifdef WIN32
			// Confirm that this winsock implimentation supports 1.1.  If it supports
			// version greater than 1.1 in addition to 1.1, it will still return 1.1
			// since that's what we inquired about.
			if (LOBYTE(ms_WSAData.wVersion) == 1 && HIBYTE(ms_WSAData.wVersion) == 1)
				{
				// Set static hook to 0 as default
				ms_callback = 0;
				
				// Set our blocking hook function
				if (WSASetBlockingHook(RProtocolBSDIP::BlockingHook) != NULL)
					{
					ms_bWSASetBlockingHook = true;
					
					// success!
					ms_bDidStartup = true;
					
					// Set protocol
					ms_prototype = RSocket::TCPIP;
					}
				else
					{
					sResult = -1;
					TRACE("RProtocolBSDIP::Hook(): Error returned by WSASetBlockingHook(): %ld\n", WSAGetLastError());
					}
				}
			else
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::Startup(): Incorrect version of WinSock DLL!\n");
				}
			#endif
			}
		else
			{
			sResult = -1;
			TRACE("RProtocolBSDIP::Startup(): Incorrect version of WinSock DLL or can't find WinSock DLL!\n");
			}
		
		// If there's an error, clean up after ourselves
		if (sResult)
			Shutdown();
		}
	
	return sResult;
	}


//////////////////////////////////////////////////////////////////////////////
//	Shutdown socket API.  This is normally called from the socket's Startup
// function.  Since we have multiple protocols using the same winsock
// code, the Startup and Shutdown will have to be called after creating
//	an RSocket and before calling Open()
//////////////////////////////////////////////////////////////////////////////
void RProtocolBSDIP::Shutdown(void)
	{
	#ifdef WIN32
	if (ms_bWSASetBlockingHook)
		{
		// Remove blocking hook
		if (WSAUnhookBlockingHook() == SOCKET_ERROR)
			TRACE("RProtocolBSDIP::Unhook(): Error returned by WSAUnhookBlockingHook(): %ld\n", WSAGetLastError());
		
		ms_bWSASetBlockingHook = false;
		}
	#endif

	if (ms_bWSAStartup)
		{
		// Cleanup winsock API
		if (WSACleanup() == SOCKET_ERROR)
			TRACE("RProtocolBSDIP::Shutdown(): Error returned by WSACleanup(): %ld\n", WSAGetLastError());
		
		ms_bWSAStartup = false;
		}
	
	// Set protocol
	ms_prototype = RSocket::NO_PROTOCOL;
	
	ms_bDidStartup = false;
	}


//////////////////////////////////////////////////////////////////////////////
// Open socket
// A return value of RSocket::errNotSupported means this protocol is
// not supported.
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::Open(							// Returns 0 if successful, non-zero otherwise
	unsigned short usPort,								// In:  Port number or 0 for any port
	short sType,											// In:  Any one RSocket::typ* enum
	short sOptionFlags,									// In:  Any combo of RSocket::opt* enums
	RSocket::BLOCK_CALLBACK callback /*NULL */)	// In:  Blocking callback (or NULL to keep current)
	{

#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;
	
	// Make sure startup was called.  Only this function needs to do this
	// because all the others check for a valid socket, which can only be
	// created via this function.
	if (ms_bDidStartup)
		{
		// Make sure socket isn't already open
		if (m_sock == INVALID_SOCKET)
			{
			// If callback was specified, use it.
            #ifdef WIN32
			if (callback != NULL)
				m_callback = callback;
			
			// Set current callback
			ms_callback = m_callback;
			#endif

			// Save type
			m_sType = sType;
			
			// Select socket type
			int iType;
			if (sType == RSocket::typStream)
				iType = SOCK_STREAM;
			else if (sType == RSocket::typDatagram)
				iType = SOCK_DGRAM;
			else
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::Open(): Invalid type specified!\n");
				}
			if (sResult == 0)
				{

				// Create socket
                #ifdef WIN32
				ms_funcnum = RSocket::OtherFunc;
                #endif

				m_sock = socket(AF_INET, iType, 0);
				if (m_sock != INVALID_SOCKET)
					{
					// Set defaults
					m_bListening = false;
					m_bConnected = false;
					m_bConnecting = false;
					
					// Check if they want blocking turned off
					if (sOptionFlags & RSocket::optDontBlock)
						{
						unsigned long ulEnableNonBlockingMode = 1;
						if (ioctlsocket(m_sock, FIONBIO, &ulEnableNonBlockingMode) == SOCKET_ERROR)
							{
							sResult = -1;
							TRACE("RProtocolBSDIP::Open(): Error setting non-blocking mode, error returned by ioctlsocket(): %ld\n", WSAGetLastError());
							}
						}

					// Check if they want coalescing turned off
					if ((sOptionFlags & RSocket::optDontCoalesce) && !sResult)
						{
						if (sType == RSocket::typStream)
							{
							int optval = 1;
							if (setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
								{
								sResult = -1;
								TRACE("RProtocolBSDIP::Open(): Error setting TCP_NODELAY, error returned by setsockopt(): %ld\n", WSAGetLastError());
								}
							}
						else
							{
							sResult = -1;
							TRACE("RPRotocolBSDIP::Open(): optDontCoalesce is only valid for typStream!\n");
							}
						}

					// Check if they want lingering turned off
					if ((sOptionFlags & RSocket::optDontWaitOnClose) && !sResult)
						{
						if (sType == RSocket::typStream)
							{
							int optval = 1;
							if (setsockopt(m_sock, SOL_SOCKET, SO_DONTLINGER, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
								{
								sResult = -1;
								TRACE("RProtocolBSDIP::Open(): Error setting SO_DONTLINGER, error returned by setsockopt(): %ld\n", WSAGetLastError());
								}
							}
						else
							{
							sResult = -1;
							TRACE("RPRotocolBSDIP::Open(): optDontWaitOnClose is only valid for typStream!\n");
							}
						}
					
					if (sResult == 0)
						{
						// Setup local address (any address is fine)
						SOCKADDR_IN addr;
						addr.sin_family = AF_INET;
						addr.sin_port = htons(usPort);
						addr.sin_addr.s_addr = INADDR_ANY;

						// Bind socket to local address
						if (bind(m_sock, (SOCKADDR*) &addr, sizeof(addr)) != SOCKET_ERROR)
							{
							// success
							}
						else
							{
							sResult = -1;
							TRACE("RProtocolBSDIP::Open(): Error returned by bind(): %ld\n", WSAGetLastError());
							}
						}

					// If anything went wrong, close the socket
					if (sResult)
						Close();
					}
				else
					{
					int iErr = WSAGetLastError();
					if (iErr == WSAEAFNOSUPPORT)
						sResult = RSocket::errNotSupported;
					else
						sResult = -1;
					TRACE("RProtocolBSDIP::Open(): Error returned by socket(): %ld\n", iErr);
					}
				}
			}
		else
			{
			sResult = -1;
			TRACE("RProtocolBSDIP::Open(): Socket already open!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::Open(): Didn't call WSAStartup()!\n");
		}

	return sResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Close socket
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::Close(							// Returns 0 if successfull, non-zero otherwise
	bool bForceNow /*= true */)						// In:  'true' means do it now, false follows normal rules
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;

	// Only close it if it's open, but don't consider it an error if it isn't
	if (m_sock != INVALID_SOCKET)
		{
		// Set current callback
		ms_callback = m_callback;
		
		// See if they want to force this to happen RIGHT NOW
		if (bForceNow && (m_sType == RSocket::typStream))
			{
			// Set the "don't linger" option, which will cause the closesocket()
			// to return immediately, but the underlying socket implimentation will
			// attempt to send all unsent data before it actually closes the connection.
			int optval = 1;
			ms_funcnum = RSocket::OtherFunc;
			if (setsockopt(m_sock, SOL_SOCKET, SO_DONTLINGER, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::Close(): Error setting SO_DONTLINGER, error returned by setsockopt(): %ld\n", WSAGetLastError());
				}
			}

		// If an error occurred, we can't do the close because it might block, which
		// is presumably the whole reason the caller asked for us to turn of lingering
		if (sResult == 0)
			{

			// Close socket
			ms_funcnum = RSocket::OtherFunc;
			if (closesocket(m_sock) != SOCKET_ERROR)
				{
				// Reset flags
				m_bListening = false;
				m_bConnected = false;
				m_bConnecting = false;
				
				// Mark it as invalid
				m_sock = INVALID_SOCKET;
				}
			else
				{
				int iErr = WSAGetLastError();
				if (iErr == WSAEWOULDBLOCK)
					sResult = RSocket::errWouldBlock;
				else
					{
					sResult = -1;
					TRACE("RProtocolBSDIP::Close(): Error returned by closesocket(): %ld\n", iErr);
					}
				}
			}
		}
	
	return sResult;
#endif
	}


////////////////////////////////////////////////////////////////////////////////
// Set socket to broadcast mode
////////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::Broadcast(void)				// Returns 0 if successfull, non-zero otherwise
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			// Set current callback
			ms_callback = m_callback;
			
			int optval = 1;
			ms_funcnum = RSocket::OtherFunc;
			if (setsockopt(m_sock, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::Broadcast(): Error setting SO_BROADCAST, error returned by setsockopt(): %ld\n", WSAGetLastError());
				}
			}
		else
			{
			sResult = -1;
			TRACE("RProtocolBSDIP::Broadcast(): Socket is already listening!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::Broadcast(): Socket is not open!\n");
		}
	
	return sResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Set socket to listen for connection requests
//
// Current winsock implimentations support a maximum of 5 queued connectons.
// Requesting more than 5 will cause this function to return an error.
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::Listen(							// Returns 0 if successfull, non-zero otherwise
	short sMaxQueued /* = 5*/)							// In:  Maximum number of queued connection requests
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			if ((sMaxQueued >= 1) && (sMaxQueued <= 5))
				{
				// Set current callback
				ms_callback = m_callback;
				
				// Current winsock implimentations are silently hardwired to support
				// no more than 5 queued connections.  They will NOT complain if you
				// set a higher number, but instead will silently set their internal
				// value to the max value (5).  Note, however, that it could be less
				// than 5, just not greater than 5.
				ms_funcnum = RSocket::OtherFunc;
				if (listen(m_sock, (int)sMaxQueued) != SOCKET_ERROR)
					{
					// Socket is listening
					m_bListening = true;
					}
				else
					{
					sResult = -1;
					TRACE("RProtocolBSDIP::Listen(): Error returned by listen(): %ld\n", WSAGetLastError());
					}
				}
			else
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::Listen(): Max queued connections must be between 1 and 5!\n");
				}
			}
		else
			{
			sResult = -1;
			TRACE("RProtocolBSDIP::Listen(): Socket is already listening!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::Listen(): Socket is not open!\n");
		}
	
	return sResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Accept request for connection
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::Accept(						// Returns 0 on success, non-zero otherwise
	RSocket::RProtocol* pProtocolClient,			// I/O: Client protocol
	RSocket::Address* paddressClient)				// Out: Client address
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (m_bListening)
			{
			// Set current callback
			ms_callback = m_callback;
			
			// Make sure client socket isn't already in use
			if (((RProtocolBSDIP*) pProtocolClient)->m_sock == INVALID_SOCKET)
				{
				// Client address will be filled in here.  Length must be set to available bytes
				// before calling accept().
				AddressIP addressClient;
				addressClient.prototype = RSocket::TCPIP;
				addressClient.lAddressLen = sizeof(addressClient.address);
				
				// Accept a client connection.  If no client is pending, then this will block.
				// Upon return, the specified address is filled in, and the specified length
				// is set to the actual length of the address (in bytes).
				ms_funcnum = RSocket::AcceptFunc;
				((RProtocolBSDIP*)pProtocolClient)->m_sock = accept(m_sock, (SOCKADDR*)&(addressClient.address), (int*)(&addressClient.lAddressLen));
				if (((RProtocolBSDIP*)pProtocolClient)->m_sock != INVALID_SOCKET)
					{
					
					// Client socket is connected
					pProtocolClient->m_bConnected = true;
					pProtocolClient->m_bConnecting = false;

					// If caller requested client address, return it now
					if (paddressClient != NULL)
						{
						// A memcpy is perfectly safe since these structs are interchangable
						memcpy(paddressClient, &addressClient, sizeof(RSocket::Address));
						}
					}
				else
					{
					int iErr = WSAGetLastError();
					if (iErr == WSAEWOULDBLOCK)
						sResult = RSocket::errWouldBlock;
					else
						{
						sResult = -1;
						TRACE("RProtocolBSDIP::AcceptClient(): Error returned by accept(): %ld\n", iErr);
						}
					}
				}
			else
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::AcceptClient(): Specified client socket is already in use!\n");
				}
			}
		else
			{
			sResult = -1;
			TRACE("RProtocolBSDIP::AcceptClient(): Socket is not listening!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::AcceptClient(): Socket is not open!\n");
		}
	
	return sResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Connect to address
//
// If the RSocket::optDontBlock option was set on this socket, then this
// function may return RSocket::errWouldBlock, which indicates that it is
// still trying to connect, but has not yet managed to do so.  In that case,
// this function should be called repeatedly (polled) until it returns either
// an actual error message other than RSocket::errWouldBlock, which would
// indicate that the connection attempt has failed, or 0, which indicates
// that it has actually connected successfully.
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::Connect(						// Returns 0 if successfull, non-zero otherwise
	RSocket::Address* paddress)						// In:  Remote address to connect to
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;

	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			// Set current callback
			ms_callback = m_callback;
				
			// Make sure socket is not already trying to connect
			if (!m_bConnecting)
				{

				// Try to connect to remote socket
				ms_funcnum = RSocket::OtherFunc;
				if (connect(m_sock, (SOCKADDR*)&(paddress->address), sizeof(SOCKADDR_IN)) != SOCKET_ERROR)
					{
					// Connected!
					m_bConnecting = false;
					m_bConnected = true;
					}
				else
					{
					int iErr = WSAGetLastError();
					if (iErr == WSAEWOULDBLOCK)
						{
						// WSAEWOULDBLOCK indicates that connect() would have blocked.  Unlike other
						// winsock functions, it will magically continue trying to connect!!!
						sResult = RSocket::errWouldBlock;
						m_bConnected = false;
						m_bConnecting = true;
						}
					else
						{
						sResult = -1;
						TRACE("RProtocolBSDIP::Connect(): Error returned by connect(): %ld\n", iErr);
						}
					}
				}
			else
				{
				// The socket is trying to connect, so we check whether it succeeded or failed

				// Create a set containing just this socket
				FD_SET set;
				FD_ZERO(&set);
				FD_SET(m_sock, &set);

				// Set time value to 0, which makes this into a polling operation
				TIMEVAL timeval = { 0, 0 };

				// Select exception/error sockets
				ms_funcnum = RSocket::SelectFunc;
				long lCount = select(0, NULL, NULL, &set, &timeval);
				if (lCount == 0)
					{
					// Create a set containing just this socket
					FD_SET set;
					FD_ZERO(&set);
					FD_SET(m_sock, &set);

					// Set time value to 0, which makes this into a polling operation
					TIMEVAL timeval = { 0, 0 };

					// Select writable sockets
					ms_funcnum = RSocket::SelectFunc;
					long lCount = select(0, NULL, &set, NULL, &timeval);
					if (lCount > 0)
						{
						// Connected!
						m_bConnecting = false;
						m_bConnected = true;
						}
					else
						{
						// Return the "would block" error, which means we're still trying to connect
						sResult = RSocket::errWouldBlock;
						}
					}
				else
					{
					// Connection attempt has failed
					m_bConnecting = false;
					m_bConnected = false;
					sResult = -1;
					TRACE("RProtocolBSDIP::Connect(): Non-blocking connection attempt has failed!\n");
					}
				}
			}
		else
			{
			sResult = -1;
			TRACE("RProtocolBSDIP::Connect(): Socket is listening -- can't connect!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::Connect(): Socket is not open!\n");
		}
	
	return sResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Send data to connected socket
// NOTE: If an error occurs, plActualBytes will return 0.
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::Send(							// Returns 0 on success, non-zero otherwise
	void* pBuf,												// In:  Pointer to data buffer
	long lNumBytes,										// In:  Number of bytes to send
	long* plActualBytes)									// Out: Actual number of bytes sent
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			if (m_bConnected)
				{
				// Set current callback
				ms_callback = m_callback;
				
				// Send data
				ms_funcnum = RSocket::OtherFunc;
				*plActualBytes = send(m_sock, (char*)pBuf, lNumBytes, 0);
				if (*plActualBytes == SOCKET_ERROR)
					{
					*plActualBytes = 0;
					int iErr = WSAGetLastError();
					if (iErr == WSAEWOULDBLOCK)
						sResult = RSocket::errWouldBlock;
					else
						{
						sResult = -1;
						TRACE("RProtocolBSDIP::Send(): Error returned by send(): %ld\n", iErr);
						}
					}
				}
			else
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::Send(): Socket not connected!\n");
				}
			}
		else
			{
			sResult = -1;
			TRACE("RProtocolBSDIP::Send(): Socket is listening -- can't send data!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::Send(): Socket is not open!\n");
		}
	
	return sResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Send data to specified address
// NOTE: If an error occurs, plActualBytes will return 0.
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::SendTo(							// Returns 0 on success, non-zero otherwise
	void* pBuf,												// In:  Pointer to data buffer
	long lNumBytes,										// In:  Number of bytes to send
	long* plActualBytes,									// Out: Actual number of bytes sent
	RSocket::Address* paddress)						// In:  Address to send to
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			// Set current callback
			ms_callback = m_callback;
			
			// Send data
			ms_funcnum = RSocket::OtherFunc;
			*plActualBytes = sendto(m_sock, (char*)pBuf, lNumBytes, 0, (SOCKADDR*)&(paddress->address), paddress->lAddressLen);
			if (*plActualBytes == SOCKET_ERROR)
				{
				*plActualBytes = 0;
				int iErr = WSAGetLastError();
				if (iErr == WSAEWOULDBLOCK)
					sResult = RSocket::errWouldBlock;
				else
					{
					sResult = -1;
					TRACE("RProtocolBSDIP::SendTo(): Error returned by sendto(): %ld\n", iErr);
					}
				}
			}
		else
			{
			sResult = -1;
			TRACE("RProtocolBSDIP::SendTo(): Socket is listening -- can't send data!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::SendTo(): Socket is not open!\n");
		}
	
	return sResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Receive data - only valid with connected sockets
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
//
// NOTE: If an error occurs, plActualBytes will return 0.
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::Receive(						// Returns 0 on success, non-zero otherwise
	void* pBuf,												// In:  Pointer to data buffer
	long lMaxBytes,										// In:  Maximum number of bytes that fit in the buffer
	long* plActualBytes)									// Out: Actual number of bytes recieved into the buffer
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			if (m_bConnected)
				{
				// Set current callback
				ms_callback = m_callback;
				
				// Receive data
				ms_funcnum = RSocket::OtherFunc;
				*plActualBytes = recv(m_sock, (char*)pBuf, lMaxBytes, 0);
				if (*plActualBytes == SOCKET_ERROR)
					{
					*plActualBytes = 0;
					int iErr = WSAGetLastError();
					if (iErr == WSAEWOULDBLOCK)
						sResult = RSocket::errWouldBlock;
					else
						{
						sResult = -1;
						TRACE("RProtocolBSDIP::Receive(): Error returned by recv(): %ld\n", iErr);
						}
					}
				}
			else
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::Receive(): Socket not connected!\n");
				}
			}
		else
			{
			sResult = -1;
			TRACE("RProtcolWinTcpip::Receive(): Socket is listening -- can't receive data!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::Receive(): Socket is not open!\n");
		}
	
	return sResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Receive data from the given address
// NOTE: If an error occurs, plActualBytes will return 0.
//////////////////////////////////////////////////////////////////////////////
short RProtocolBSDIP::ReceiveFrom(					// Returns 0 on success, non-zero otherwise
	void* pBuf,												// In:  Pointer to data buffer
	long lMaxBytes,										// In:  Maximum bytes that can fit in the buffer
	long* plActualBytes,									// Out: Actual number of bytes recieved into buffer
	RSocket::Address* paddress)						// Out: Source address returned here
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			// Set current callback
			ms_callback = m_callback;
			
			// Source address
			AddressIP addr;
			addr.prototype = RSocket::TCPIP;
			addr.lAddressLen = sizeof(addr.address);
			
			// Receive data
			ms_funcnum = RSocket::OtherFunc;
			*plActualBytes = recvfrom(m_sock, (char*)pBuf, lMaxBytes, 0, (SOCKADDR*)&(addr.address), (int*)(&addr.lAddressLen));
			if (*plActualBytes != SOCKET_ERROR)
				{
				// If caller requested source address, return it now
				if (paddress != NULL)
					{
					// A memcpy is perfectly safe since these structs are interchangable
					memcpy(paddress, &addr, sizeof(RSocket::Address));
					}
				}
			else
				{
				*plActualBytes = 0;
				int iErr = WSAGetLastError();
				if (iErr == WSAEWOULDBLOCK)
					sResult = RSocket::errWouldBlock;
				else
					{
					sResult = -1;
					TRACE("RProtocolBSDIP::ReceiveFrom(): Error returned by recvfrom(): %ld\n", iErr);
					}
				}
			}
		else
			{
			sResult = -1;
			TRACE("RProtocolBSDIP::ReceiveFrom(): Socket is listening -- can't receive data!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::ReceiveFrom(): Socket is not open!\n");
		}
	
	return sResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Check if connection can be accepted without blocking
//////////////////////////////////////////////////////////////////////////////
bool RProtocolBSDIP::CanAcceptWithoutBlocking(void)
	{
#if PLATFORM_UNIX
    return(false);
#else
	bool bResult = false;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (m_bListening)
			{
			// Set current callback
			ms_callback = m_callback;
			
			// Create a set containing just this socket
			FD_SET set;
			FD_ZERO(&set);
			FD_SET(m_sock, &set);
			
			// Set time value to 0, which makes this into a polling operation
			TIMEVAL timeval = { 0, 0 };
			
			// Select listening sockets that have actual connections pending
			ms_funcnum = RSocket::SelectFunc;
			long lCount = select(0, &set, NULL, NULL, &timeval);
			if (lCount > 0)
				bResult = true;
			}
		else
			{
			//sResult = -1;
			TRACE("RProtocolBSDIP::CanAcceptWithoutBlocking(): Socket is not listening!\n");
			}
		}
	else
		{
		//sResult = -1;
		TRACE("RProtocolBSDIP::CanAcceptWithoutBlocking(): Socket is not open!\n");
		}
	
	return bResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Check if socket can send without blocking.  There is no way of knowing
// how long this will be true, especially in a multi-threading environment
//////////////////////////////////////////////////////////////////////////////
bool RProtocolBSDIP::CanSendWithoutBlocking(void)
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	bool bResult = false;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			// Set current callback
			ms_callback = m_callback;
			
			// Create a set containing just this socket
			FD_SET set;
			FD_ZERO(&set);
			FD_SET(m_sock, &set);
			
			// Set time value to 0, which makes this into a polling operation
			TIMEVAL timeval = { 0, 0 };
			
			// Select writable sockets
			ms_funcnum = RSocket::SelectFunc;
			long lCount = select(0, NULL, &set, NULL, &timeval);
			if (lCount > 0)
				bResult = true;
			}
		else
			{
			//sResult = -1;
			TRACE("RProtocolBSDIP::CanSendWithoutBlocking(): Socket is listening -- can't send data!\n");
			}
		}
	else
		{
		//sResult = -1;
		TRACE("RProtocolBSDIP::CanSendWithoutBlocking(): Socket is not open!\n");
		}
	
	return bResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Check if socket can be read without blocking.  For datagrams 'true' means 
// queued data is available.  For streams 'true' means the connection has been
// closed.  If it was grecefully closed, the next read will return 0 bytes read.
// If it was aborted, the next read will return with an error.
//////////////////////////////////////////////////////////////////////////////
bool RProtocolBSDIP::CanReceiveWithoutBlocking(void)
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	bool bResult = false;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			// Set current callback
			ms_callback = m_callback;
			
			// Create a set containing just this socket
			FD_SET set;
			FD_ZERO(&set);
			FD_SET(m_sock, &set);
			
			// Set time value to 0, which makes this into a polling operation
			TIMEVAL timeval = { 0, 0 };
			
			// Select readable sockets
			ms_funcnum = RSocket::SelectFunc;
			long lCount = select(0, &set, NULL, NULL, &timeval);
			if (lCount > 0)
				bResult = true;
			}
		else
			{
			//sResult = -1;
			TRACE("RProtocolBSDIP::CanReceiveWithoutBlocking(): Socket is listening -- can't receive data!\n");
			}
		}
	else
		{
		//sResult = -1;
		TRACE("RProtocolBSDIP::CanReceiveWithoutBlocking(): Socket is not open!\n");
		}
	
	return bResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// Check the number of bytes of data receivable without blocking.  For
// datagrams this returns the size of the next queued datagram.  For streams
// this returns the total amount of data that can be read with a single
// Receive() which is normally equal to the total amount of queued data.
//////////////////////////////////////////////////////////////////////////////
long RProtocolBSDIP::CheckReceivableBytes(void)
	{
#if PLATFORM_UNIX
    return(0);  // !!! FIXME
#else
	long lResult = 0;
	
	if (m_sock != INVALID_SOCKET)
		{
		if (!m_bListening)
			{
			// Set current callback
			ms_callback = m_callback;
			
			// Get amount of data available for reading
			ms_funcnum = RSocket::OtherFunc;
			if (ioctlsocket(m_sock, FIONREAD, (unsigned long*)&lResult) == SOCKET_ERROR)
				{
				lResult = 0;
				TRACE("RProtocolBSDIP::GetAvailableForReceive(): Error returned by ioctrlsocket(): %ld\n", WSAGetLastError());
				}
			}
		else
			{
			//sResult = -1;
			TRACE("RProtocolBSDIP::CanReceiveWithoutBlocking(): Socket is listening -- can't receive data!\n");
			}
		}
	else
		{
		//sResult = -1;
		TRACE("RProtocolBSDIP::CanReceiveWithoutBlocking(): Socket is not open!\n");
		}
	
	return lResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// check status of protocol and this socket.
//////////////////////////////////////////////////////////////////////////////
bool RProtocolBSDIP::IsError(void)
	{
#if PLATFORM_UNIX
    return(true);  // !!! FIXME
#else
	bool bResult = false;
	
	if (m_sock != INVALID_SOCKET)
		{
		// Set current callback
		ms_callback = m_callback;
		
		// Create a set containing just this socket
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(m_sock, &set);
		
		// Set time value to 0, which makes this into a polling operation
		TIMEVAL timeval = { 0, 0 };
		
		// Select exception/error sockets
		ms_funcnum = RSocket::SelectFunc;
		long lCount = select(0, NULL, NULL, &set, &timeval);
		if (lCount > 0)
			bResult = true;
		}
	
	return bResult;
#endif
	}


//////////////////////////////////////////////////////////////////////////////
// SetCallback
//////////////////////////////////////////////////////////////////////////////
void RProtocolBSDIP::SetCallback(
	RSocket::BLOCK_CALLBACK callback)
	{
	m_callback = callback;
	}


//////////////////////////////////////////////////////////////////////////////
// GetCallback
//////////////////////////////////////////////////////////////////////////////
RSocket::BLOCK_CALLBACK RProtocolBSDIP::GetCallback(void)
	{
	return m_callback;
	}


//////////////////////////////////////////////////////////////////////////////
// GetMaxDatagramSize
//////////////////////////////////////////////////////////////////////////////
/* static */
short RProtocolBSDIP::GetMaxDatagramSize(			// Returns zero on success, non-zero otherwise
	long* plSize)											// Out: Maximum datagram size in bytes
	{
	short sResult = 0;
	
    #ifdef WIN32
	if (ms_bDidStartup)
		{
		*plSize = (long)ms_WSAData.iMaxUdpDg;
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::GetMaxDatagramSize(): Never called Startup()!\n");
		}
    #else
    *plSize = 1024;   // uh, sure.
    #endif

	return sResult;
	}


//////////////////////////////////////////////////////////////////////////////
// Get maximum number of sockets.  This may be a system "global" value, which
// means that if other applications are using sockets, then the number
// available to this application may be lower than the returned value.
//////////////////////////////////////////////////////////////////////////////
/* static */
short RProtocolBSDIP::GetMaxSockets(				// Returns 0 if successfull, non-zero otherwise
	long* plNum)											// Out: maximum number of sockets
	{
	short sResult = 0;

	#ifdef WIN32
	if (ms_bDidStartup)
		{
		*plNum = (long)ms_WSAData.iMaxSockets;
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::GetMaxSockets(): Never called Startup()!\n");
		}
	#else
	*plNum = 25;  // uh...ok.
	#endif
	return sResult;
	}


//////////////////////////////////////////////////////////////////////////////
// Get the address of the specified host from the string provided which may be
// a name or address
//////////////////////////////////////////////////////////////////////////////
/* static */
short RProtocolBSDIP::GetAddress(					// Returns 0 if successfull, non-zero otherwise
	char* pszName,											// In:  Host's name or dotted address (x.x.x.x)
	USHORT usPort,											// In:  Host's port number
	RSocket::Address* paddress)						// Out: Address
	{
#if PLATFORM_UNIX
    return(-1);  // !!! FIXME
#else
	short sResult = 0;
	
	if (ms_bDidStartup)
		{
		// Cast address to IP address to make it easier to work with
		AddressIP* pip = (AddressIP*)paddress;
		
		// If the string contains only digits and/or dots and there's at least one dot
		// then we assume it's a dotted address.
		if ((strspn(pszName, "0123456789.") >= strlen(pszName)) && (strchr(pszName, '.') != NULL))
			{
			// Convert dotted address into value (returned in network order!)
			unsigned long ulAddr = inet_addr(pszName);
			if (ulAddr != INADDR_NONE)
				{
				// Fill in the address
				pip->prototype = RSocket::TCPIP;
				pip->lAddressLen = sizeof(pip->address);
				pip->address.sin_family = AF_INET;
				pip->address.sin_addr.S_un.S_addr = ulAddr;
				pip->address.sin_port = htons(usPort);
				}
			else
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::GetAddress(): Invalid dotted address: %s\n", pszName);
				}
			}
		else
			{
			// Lookup address based on specified name
			ms_funcnum = RSocket::OtherFunc;
			HOSTENT* phost = gethostbyname(pszName);
			if (phost != NULL)
				{
				// Fill in the address
				pip->prototype = RSocket::TCPIP;
				pip->lAddressLen = sizeof(pip->address);
				pip->address.sin_family = phost->h_addrtype;
				memcpy((char*)&(pip->address.sin_addr), phost->h_addr_list[0], phost->h_length);
				pip->address.sin_port = htons(usPort);
				}
			else
				{
				sResult = -1;
				TRACE("RProtocolBSDIP::GetAddress(): Error returned by gethostbyname(): %ld\n", WSAGetLastError());
				}
			}
		}
	else
		{
		sResult = -1;
		TRACE("RProtocolBSDIP::GetHostAddress(): Never called Startup()!\n");
		}
	
	return sResult;
#endif
	}


////////////////////////////////////////////////////////////////////////////////
// Create broadcast address using specified port
////////////////////////////////////////////////////////////////////////////////
// static
void RProtocolBSDIP::CreateBroadcastAddress(
	unsigned short usPort,								// In:  Port to broadcast to
	RSocket::Address* paddress)						// Out: Broadcast address returned here
	{
	// Cast address to IP address to make it easier to work with
	AddressIP* pip = (AddressIP*)paddress;
	
	// Create broadcast address using specified port
	pip->prototype = RSocket::TCPIP;
	pip->lAddressLen = sizeof(pip->address);
	pip->address.sin_family = AF_INET;
	pip->address.sin_port = htons(usPort);

    #ifdef WIN32
	pip->address.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);
    #else
	pip->address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    #endif
	}


//////////////////////////////////////////////////////////////////////////////
// Get the specified address' current port number
//////////////////////////////////////////////////////////////////////////////
/* static */
unsigned short RProtocolBSDIP::GetAddressPort(	// Returns port number
	RSocket::Address* paddress)						// In:  Address from which to get port
	{
	PSOCKADDR_IN pin = (PSOCKADDR_IN) &(paddress->address);
	
	return ntohs(pin->sin_port);
	}


//////////////////////////////////////////////////////////////////////////////
// Set the specified address' port to the specified value
//////////////////////////////////////////////////////////////////////////////
/* static */
void RProtocolBSDIP::SetAddressPort(
	USHORT usPort,											// In:  New port number
	RSocket::Address* paddress)						// I/O: Address who's port is to be set
	{
	PSOCKADDR_IN pin = (PSOCKADDR_IN) &(paddress->address);
	
	pin->sin_port = htons(usPort);
	}


////////////////////////////////////////////////////////////////////////////////
// This is a blocking-hook callback function.
// The winsock code will call this repeatedly while the return value is
// non-zero, but the documentation implies that it doesn't do anything else
// while calling this, so it seems like a better idea to return 0 all the time.
////////////////////////////////////////////////////////////////////////////////
/* static */
#ifdef WIN32
int CALLBACK RProtocolBSDIP::BlockingHook(void)
	{
	// !!!
	// The only Windows Socket API function you can call from this
	// function is WSACancelBlockingCall()
	// !!!
	
	// Call user-installed callback.  If return value is non-zero, cancel
	// the current blocking call.
	if (ms_callback != 0)
		{
		if ((*ms_callback)() != 0)
			{
			// Cancelling accept() or select() is fine --  only that particular
			// operation will fail, and the socket itself will be unaffected.
			// Cancelling any other operation will leave the socket in an unknown/
			// unstable state.  The only function you can call after cancelling
			// operations other than accept() or select() is closesocket().
			WSACancelBlockingCall();
			}
		}
	
	return 0;
	}
#endif


//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
