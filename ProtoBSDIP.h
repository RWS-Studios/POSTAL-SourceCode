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
				int32_t						lAddressLen;		// Actual address length (always <= MaxAddressSize)
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
		int16_t		m_sType;									// Type of socket (RSocket::typ*)

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
		virtual int16_t Open(										// Returns 0 if connection was opened
			uint16_t usPort,								// In:  Port number on which to make a connection
			int16_t sType,											// In:  Any one RSocket::typ* enum
			int16_t sOptionFlags,									// In:  Any combo of RSocket::opt* enums
			RSocket::BLOCK_CALLBACK callback = NULL);		// In:  Blocking callback (or NULL to keep current callback)

		// Close a connection
		virtual int16_t Close(										// Returns 0 if successfull, non-zero otherwise
			bool bForceNow = true);								// In:  'true' means do it now, false follows normal rules

		// Set socket to broadcast mode
		virtual int16_t Broadcast(void);						// Returns 0 if successfull, non-zero otherwise

		// Listen for connection requests
		virtual int16_t Listen(									// Returns 0 if listen port established
			int16_t sMaxQueued);									// In:  Maximum number of queueued connection requests 

		// Accept request for connection
		virtual int16_t Accept(									// Returns 0 if accepted
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
		virtual int16_t Connect(									// Returns 0 if connected
			RSocket::Address* paddress);						// In:  Remote address to connect to

		// Send data - only valid with connected sockets
		virtual int16_t Send(										// Returns 0 if data was sent
			void * pBuf,											// In:  Pointer to data buffer
			int32_t lNumBytes,										// In:  Number of bytes to send
			int32_t *plActualBytes);								// Out: Acutal number of bytes sent

		// SendTo - send data to specified address - for unconnected sockets
		virtual int16_t SendTo(									// Returns 0 if data was sent
			void* pBuf,												// In:  Pointer to data buffer
			int32_t lNumBytes,										// In:  Number of bytes to send
			int32_t* plActualBytes,									// Out: Actual number of bytes sent
			RSocket::Address* paddress);						// In:  Address to send to

		// Receive data - only valid for connected sockets
		virtual int16_t Receive(									// Returns 0 if data was received
			void* pBuf,												// In:  Pointer to data buffer
			int32_t lMaxBytes,										// In:  Maximum number of bytes that fit in the buffer
			int32_t* plActualBytes);								// Out: Actual number of bytes received into buffer

		// RecieveFrom - receive data from given address
		virtual int16_t ReceiveFrom(								// Returns 0 if data was received
			void* pBuf,												// In:  Pointer to data buffer
			int32_t lMaxBytes,										// In:  Maximum bytes that can fit in buffer
			int32_t* plActualBytes,									// Out:  Actual number of bytes received into buffer
			RSocket::Address* paddress);						// Out: Source address returned here

		// Check if connection can be accepted without blocking
		virtual bool CanAcceptWithoutBlocking(void);

		// Check if connection can send without blocking
		virtual bool CanSendWithoutBlocking(void);

		// Check if connection can receive without blocking
		virtual bool CanReceiveWithoutBlocking(void);

		// See how much data can be received without blocking
		virtual int32_t CheckReceivableBytes(void);

		// Report error status
		virtual bool IsError(void);

		// Set callback function
		virtual void SetCallback(
			RSocket::BLOCK_CALLBACK callback);

		// Get callback function
		virtual RSocket::BLOCK_CALLBACK GetCallback(void);

		// Startup - Do any one time initialization needed for the protocol
		static int16_t Startup(void);							// Return 0 if there were no errors on startup

		// Shutdown - Do any clean up after all objects of a protocol have shut down
		static void Shutdown(void);

		// Get the maximum datagram size supported by this socket
		static int16_t GetMaxDatagramSize(						// Returns 0 if info is available
			int32_t* plSize);											// Out: Maximum datagram size (in bytes)

		// Get maximum number of sockets
		static int16_t GetMaxSockets(							// Returns 0 if successfull, non-zero otherwise
			int32_t* plNum);

		// Get address from the socket
		static int16_t GetAddress(								// Returns 0 if successfull, non-zero otherwise
			char* pszName,											// In:  Host's name or dotted address
			uint16_t usPort,											// In:  Host's port number
			RSocket::Address* paddress);						// Out: Address

		// Create broadcast address using specified port
		static void CreateBroadcastAddress(
			uint16_t usPort,								// In:  Port to broadcast to
			RSocket::Address* paddress);						// Out: Broadcast address returned here

		// Get port from address
		static uint16_t GetAddressPort(				// Returns port number
			RSocket::Address* paddress);						// In:  Address from which to get port

		// Set port for address
		static void SetAddressPort(
			uint16_t usPort,											// In: New port number
			RSocket::Address* paddress);						// I/O:Address who's port is to be set

	protected:

		#ifdef WIN32
		// This is a blocking-hook callback function.
		static intptr_t CALLBACK BlockingHook(void);
		#endif

		// Do the initialization stuff
		// NOTE: Derived classes MUST call base class implimentation!
		void Init(void);
};

#endif //PROTOBSDIP_H

//////////////////////////////////////////////////////////////////////////////
// eof
//////////////////////////////////////////////////////////////////////////////
