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
// ProtoBSDIP.h
// Project: Postal
// 
// History:
//		08/04/97 BRH	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// This is the BSD sockets/winsock tcpip plugin protocol for our sockets
//  interface.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef PROTOBSDIP_H
#define PROTOBSDIP_H

#ifdef WIN32
#include <winsock.h>
#else
// (that should just about cover it... --ryan.)
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

// WinSock will define these, but BSD sockets don't...
#if 0 //PLATFORM_MACOSX
typedef int socklen_t;
#endif

typedef struct sockaddr_in SOCKADDR_IN;
typedef SOCKADDR_IN *PSOCKADDR_IN;
typedef int SOCKET;
typedef struct hostent HOSTENT;
typedef in_addr IN_ADDR;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct linger LINGER;
typedef struct timeval TIMEVAL;
typedef socklen_t SOCKLEN;
typedef int WSADATA;  // not used in BSD sockets.
#define WSAStartup(a, b) (0)
#define WSACleanup() (0)
#define WSAGetLastError() (errno)
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#define ioctlsocket(s, t, v) ioctl(s, t, v)
#endif

#include "socket.h"


class RProtocolBSDIP : public RSocket::RProtocol
{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:
	
		// This protocol's specific address, which MUST BE EXACTLY THE SAME SIZE
		// as RProtocol::Address, and the first fields prior to the address field
		// MUST BE THE SAME as RProtocol::Address.  Any necessary padding should be
		// added after the address field.
		typedef struct tAddressIP
			{
			public:
				RSocket::ProtoType	prototype;			// Type of protocol
				long						lAddressLen;		// Actual address length (always <= MaxAddressSize)
				SOCKADDR_IN				address;				// Address

				bool operator==(const tAddressIP& rhs) const
					{
					// Note that we ignore the zero stuff, which doesn't matter if it's different
					if ( (prototype == rhs.prototype) &&
						  (lAddressLen == rhs.lAddressLen) &&
						  (address.sin_family == rhs.address.sin_family) &&
						  (address.sin_port == rhs.address.sin_port) &&
						  (memcmp(&address.sin_addr, &rhs.address.sin_addr, sizeof(address.sin_addr)) == 0) )
						return true;
					return false;
					}
			} AddressIP;

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	public:
		SOCKET	m_sock;									// Socket
		short		m_sType;									// Type of socket (RSocket::typ*)

	//------------------------------------------------------------------------------
	// Static Variables
	//------------------------------------------------------------------------------
	public:
		static bool ms_bDidStartup;					// Whether Startup was called successfully
		static bool ms_bWSAStartup;					// Whether WSAStartup() was called
		static WSADATA ms_WSAData;						// Data regarding current socket implimentation
		#ifdef WIN32
		static bool ms_bWSASetBlockingHook;			// Whether WSASetBlockingHook() was called
		static RSocket::BLOCK_CALLBACK ms_callback;	// Blocking hook callback
		static RSocket::FuncNum ms_funcnum;			// Which socket function, if any, is being executed
		#endif

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:

		// Constructor
		RProtocolBSDIP();

		// Destructor
		virtual ~RProtocolBSDIP();

		// Restart the object without deleting it.
		// NOTE: Derived classes MUST call base class implimentation!
		void Reset(void);

		// Open a new connection
		// A return value of RSocket::errNotSupported means this protocol is
		// not supported.
		virtual short Open(										// Returns 0 if connection was opened
			unsigned short usPort,								// In:  Port number on which to make a connection
			short sType,											// In:  Any one RSocket::typ* enum
			short sOptionFlags,									// In:  Any combo of RSocket::opt* enums
			RSocket::BLOCK_CALLBACK callback = NULL);		// In:  Blocking callback (or NULL to keep current callback)

		// Close a connection
		virtual short Close(										// Returns 0 if successfull, non-zero otherwise
			bool bForceNow = true);								// In:  'true' means do it now, false follows normal rules

		// Set socket to broadcast mode
		virtual short Broadcast(void);						// Returns 0 if successfull, non-zero otherwise

		// Listen for connection requests
		virtual short Listen(									// Returns 0 if listen port established
			short sMaxQueued);									// In:  Maximum number of queueued connection requests 

		// Accept request for connection
		virtual short Accept(									// Returns 0 if accepted
			RSocket::RProtocol* pProtocol,					// Out: Client's protocol
			RSocket::Address* paddress);						// Out: Client's address returned here

		// Connect to address.
		// If the RSocket::optDontBlock option was set on this socket, then this
		// function may return RSocket::errWouldBlock, which indicates that it is
		// still trying to connect, but has not yet managed to do so.  In that case,
		// this function should be called repeatedly (polled) until it returns either
		// an actual error message other than RSocket::errWouldBlock, which would
		// indicate that the connection attempt has failed, or 0, which indicates
		// that it has actually connected successfully.
		virtual short Connect(									// Returns 0 if connected
			RSocket::Address* paddress);						// In:  Remote address to connect to

		// Send data - only valid with connected sockets
		virtual short Send(										// Returns 0 if data was sent
			void * pBuf,											// In:  Pointer to data buffer
			long lNumBytes,										// In:  Number of bytes to send
			long *plActualBytes);								// Out: Acutal number of bytes sent

		// SendTo - send data to specified address - for unconnected sockets
		virtual short SendTo(									// Returns 0 if data was sent
			void* pBuf,												// In:  Pointer to data buffer
			long lNumBytes,										// In:  Number of bytes to send
			long* plActualBytes,									// Out: Actual number of bytes sent
			RSocket::Address* paddress);						// In:  Address to send to

		// Receive data - only valid for connected sockets
		virtual short Receive(									// Returns 0 if data was received
			void* pBuf,												// In:  Pointer to data buffer
			long lMaxBytes,										// In:  Maximum number of bytes that fit in the buffer
			long* plActualBytes);								// Out: Actual number of bytes received into buffer

		// RecieveFrom - receive data from given address
		virtual short ReceiveFrom(								// Returns 0 if data was received
			void* pBuf,												// In:  Pointer to data buffer
			long lMaxBytes,										// In:  Maximum bytes that can fit in buffer
			long* plActualBytes,									// Out:  Actual number of bytes received into buffer
			RSocket::Address* paddress);						// Out: Source address returned here

		// Check if connection can be accepted without blocking
		virtual bool CanAcceptWithoutBlocking(void);

		// Check if connection can send without blocking
		virtual bool CanSendWithoutBlocking(void);

		// Check if connection can receive without blocking
		virtual bool CanReceiveWithoutBlocking(void);

		// See how much data can be received without blocking
		virtual long CheckReceivableBytes(void);

		// Report error status
		virtual bool IsError(void);

		// Set callback function
		virtual void SetCallback(
			RSocket::BLOCK_CALLBACK callback);

		// Get callback function
		virtual RSocket::BLOCK_CALLBACK GetCallback(void);

		// Startup - Do any one time initialization needed for the protocol
		static short Startup(void);							// Return 0 if there were no errors on startup

		// Shutdown - Do any clean up after all objects of a protocol have shut down
		static void Shutdown(void);

		// Get the maximum datagram size supported by this socket
		static short GetMaxDatagramSize(						// Returns 0 if info is available
			long* plSize);											// Out: Maximum datagram size (in bytes)

		// Get maximum number of sockets
		static short GetMaxSockets(							// Returns 0 if successfull, non-zero otherwise
			long* plNum);

		// Get address from the socket
		static short GetAddress(								// Returns 0 if successfull, non-zero otherwise
			char* pszName,											// In:  Host's name or dotted address
			USHORT usPort,											// In:  Host's port number
			RSocket::Address* paddress);						// Out: Address

		// Create broadcast address using specified port
		static void CreateBroadcastAddress(
			unsigned short usPort,								// In:  Port to broadcast to
			RSocket::Address* paddress);						// Out: Broadcast address returned here

		// Get port from address
		static unsigned short GetAddressPort(				// Returns port number
			RSocket::Address* paddress);						// In:  Address from which to get port

		// Set port for address
		static void SetAddressPort(
			USHORT usPort,											// In: New port number
			RSocket::Address* paddress);						// I/O:Address who's port is to be set

	protected:

		#ifdef WIN32
		// This is a blocking-hook callback function.
		static int CALLBACK BlockingHook(void);
		#endif

		// Do the initialization stuff
		// NOTE: Derived classes MUST call base class implimentation!
		void Init(void);
};

#endif //PROTOBSDIP_H

//////////////////////////////////////////////////////////////////////////////
// eof
//////////////////////////////////////////////////////////////////////////////

