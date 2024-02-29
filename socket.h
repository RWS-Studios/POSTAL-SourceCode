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
// s.h
// Project: Postal
//
//	History:
//		08/04/97 BRH	Started the socket over again with plugin protocols
//
//		08/12/97 MJR	Added mac protocols.
//
//		08/20/97 MJR	Added support for setting whether socket blocks or not.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SOCKET_H
#define SOCKET_H

#include "RSPiX.h"

////////////////////////////////////////////////////////////////////////////////
//
// NOTE: There's more #include's at the end of this file!
//
// This was necessary because all the "plugin" protocols needed the definition
// of the RSocket class, but at the same time, we didn't want make everyone
// that uses RSocket to #include the invidual protocol header files.  No big
// deal -- it just seemed worth explaining why the #include's are at the end.
//
////////////////////////////////////////////////////////////////////////////////

// Use this to enable APPLETALK protocol when support for it has been added
#define ENABLE_APPLETALK 0


class RSocket
{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:

		// Blocking callback
		typedef short (*BLOCK_CALLBACK) (void);

		// Supported protocols
		typedef enum
			{
			NO_PROTOCOL = 0,				// No protocol (MUST BE 0!!!)

			FirstProtocol = 1,			// First protocol (used when iterating protocol types)

			TCPIP			= 1,			// WinSock or BSD Sockets TCP/IP

			NumProtocols					// Number of protocol types
			} ProtoType;

		// Miscellaneous stuff
		enum
			{
			// Socket types
			typStream				= 0,				// Stream-oriented connection
			typDatagram				= 1,				// Datagram-oriented connection

			// Available socket options
			optDontBlock			= 0x0001,		// Don't block
			optDontWaitOnClose	= 0x0002,		// Don't wait on Close()	(applies to typStream only)
			optDontCoalesce		= 0x0004,		// Dont coalesce data		(applies to typStream only)

			// Special error return values
			errWouldBlock			= 1000,			// Function would have blocked, but "optDontBlock" was set
			errNotSupported		= 1001,			// Protocol is not supported

			// This is the maximum size of any protocol's address
			MaxAddressSize = 16
			};

		// This is a generic address
		typedef struct tAddress
			{
			public:
				ProtoType	prototype;						// Type of protocol
				long			lAddressLen;					// Actual address length (depends on protocol, always <= MaxAddressSize)
				char			address[MaxAddressSize];	// Raw address (interpretation depends on protocol type)

				// Note: operator== is defined at global scope, outside this class definition,
				// because it needs access to each protocol's specific address type, which isn't
				// defined until after this class.

			void Reset(void)
				{
				prototype = NO_PROTOCOL;
				lAddressLen = 0;
				memset(address, 0, sizeof(address));
				}

			} Address;

		// Value indicating which socket function, if any, is currenly being executed.
		// This allows a callback to determine which function is being executed, which
		// may help it decide what to do.  Under WinSock, aborting certain functions
		// causes the socket to become unstable.  See WinSock docs for details.
		typedef enum
			{
			NoFunc,											// none
			SelectFunc,										// select()
			AcceptFunc,										// accept()
			OtherFunc										// all other functions
			} FuncNum;

	//------------------------------------------------------------------------------
	// Protocol class, which is the basis of the "plug-in" architecture
	//------------------------------------------------------------------------------
	public:
		class RProtocol
			{
			//------------------------------------------------------------------------------
			// Types, enums, etc.
			//------------------------------------------------------------------------------
			public:

			//------------------------------------------------------------------------------
			// Variables
			//------------------------------------------------------------------------------
			public:
				bool m_bListening;								// Whether socket is listening (true) or not (false)
				bool m_bConnecting;								// Whether socket is trying to connect (true) or not (false)
				bool m_bConnected;								// Whether socket is connected (true) or not (false)
				BLOCK_CALLBACK m_callback;						// Callback (defaults to 0, which means none)

			//------------------------------------------------------------------------------
			// Static Variables
			//------------------------------------------------------------------------------
			public:
				static ProtoType ms_prototype;				// Current protocol type (there can be only one "current" type)
			
			//------------------------------------------------------------------------------
			// Functions
			//------------------------------------------------------------------------------
			public:
				// Constructor
				RProtocol()
					{
					Init();
					}

				// Destructor
				virtual ~RProtocol()
					{
					}

				// Restart the object without deleting it.
				// NOTE: Derived classes MUST call base class implimentation!
				virtual void Reset(void)
					{
					Init();
					}

				// Open a new connection.
				// A return value of RSocket::errNotSupported means this protocol is
				// not supported.
				virtual short Open(										// Returns 0 if connection was opened
					unsigned short usPort,								// In:  Port number on which to make a connection
					short sType,											// In:  Any one RSocket::typ* enum
					short sOptionFlags,									// In:  Any combo of RSocket::opt* enums
					BLOCK_CALLBACK callback = NULL)					// In:  Blocking callback (or NULL to keep current callback)
					= 0;

				// Close a connection
				virtual short Close(										// Returns 0 if successfull, non-zero otherwise
					bool bForceNow = true)								// In:  'true' means do it now, false follows normal rules
					= 0;

				// Set socket to broadcast mode
				virtual short Broadcast(void)							// Returns 0 if successfull, non-zero otherwise
					= 0;

				// Listen for connection requests
				virtual short Listen(									// Returns 0 if listen port established
					short sMaxQueued)										// In:  Maximum number of queueued connection requests 
					= 0;

				// Accept request for connection
				virtual short Accept(									// Returns 0 if accepted
					RProtocol* pProtocol,								// Out: Client's protocol
					Address* paddress)									// Out: Client's address returned here
					= 0;

				// Connect to address.
				// If the RSocket::optDontBlock option was set on this socket, then this
				// function may return RSocket::errWouldBlock, which indicates that it is
				// still trying to connect, but has not yet managed to do so.  In that case,
				// this function should be called repeatedly (polled) until it returns either
				// an actual error message other than RSocket::errWouldBlock, which would
				// indicate that the connection attempt has failed, or 0, which indicates
				// that it has actually connected successfully.
				virtual short Connect(									// Returns 0 if connected
					Address* paddress)									// In:  Remote address to connect to
					= 0;

				// Send data - only valid with connected sockets
				virtual short Send(										// Returns 0 if data was sent
					void * pBuf,											// In:  Pointer to data buffer
					long lNumBytes,										// In:  Number of bytes to send
					long *plActualBytes)									// Out: Acutal number of bytes sent
					= 0;

				// SendTo - send data to specified address - for unconnected sockets
				virtual short SendTo(									// Returns 0 if data was sent
					void* pBuf,												// In:  Pointer to data buffer
					long lNumBytes,										// In:  Number of bytes to send
					long* plActualBytes,									// Out: Actual number of bytes sent
					Address* paddress)									// In:  Address to send to
					= 0;

				// Receive data - only valid for connected sockets
				virtual short Receive(									// Returns 0 if data was received
					void* pBuf,												// In:  Pointer to data buffer
					long lMaxBytes,										// In:  Maximum number of bytes that fit in the buffer
					long* plActualBytes)									// Out: Actual number of bytes received into buffer
					= 0;

				// RecieveFrom - receive data from given address
				virtual short ReceiveFrom(								// Returns 0 if data was received
					void* pBuf,												// In:  Pointer to data buffer
					long lMaxBytes,										// In:  Maximum bytes that can fit in buffer
					long* plActualBytes,									// Out:  Actual number of bytes received into buffer
					Address* paddress)									// Out: Source address returned here
					= 0;

				// Status functions
				virtual bool IsError(void) = 0;
				virtual bool CanAcceptWithoutBlocking(void) = 0;
				virtual bool CanSendWithoutBlocking(void) = 0;
				virtual bool CanReceiveWithoutBlocking(void) = 0;
				virtual long CheckReceivableBytes(void) = 0;

				// Set callback function
				virtual void SetCallback(BLOCK_CALLBACK callback) = 0;

				// Get callback function
				virtual BLOCK_CALLBACK GetCallback(void) = 0;

			protected:
				// Init
				// NOTE: Derived classes MUST call base class implimentation!
				virtual void Init(void)
					{
					m_bListening = false;
					m_bConnected = false;
					m_callback = 0;
					}
			};

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	public:
		RProtocol*						m_pProtocol;			// Pointer to protocol object

	//------------------------------------------------------------------------------
	// Static Variables
	//------------------------------------------------------------------------------
	protected:
		static bool						ms_bDidStartup;		// Whether Startup() was called successfully
		static bool						ms_bAutoShutdown;		// Whether to call Shutdown() automatically
		static short					ms_sNumSockets;		// Number of sockets in existance
		static RSocket::ProtoType	ms_prototype;			// Current protocol (can only be one "current" protocol)
		static char*					ms_apszProtoNames[];	// String names corresponding to RSocket::ProtoType values

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		RSocket();

		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~RSocket();


		////////////////////////////////////////////////////////////////////////////////
		// Reset socket to its post-construction state
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void);


		////////////////////////////////////////////////////////////////////////////////
		// Open socket in datagram or stream mode.
		//
		// If the current protocol is not supported, this function returns the value
		// RSocket::errNotSupported.
		////////////////////////////////////////////////////////////////////////////////
		short Open(													// Returns 0 if successfull, non-zero otherwise
			unsigned short usPort,								// In:  Port number or 0 for any port
			short sType,											// In:  Any one RSocket::typ* enum
			short sOptionFlags,									// In:  Any combo of RSocket::opt* enums
			BLOCK_CALLBACK callback = NULL);					// In:  Blocking callback (or NULL to keep current callback)


		////////////////////////////////////////////////////////////////////////////////
		// Close socket
		////////////////////////////////////////////////////////////////////////////////
		short Close(												// Returns 0 if successfull, non-zero otherwise
			bool bForceNow = false);							// In:  'true' means do it now, false follows normal rules


		////////////////////////////////////////////////////////////////////////////////
		// Set socket to broadcast mode
		//
		// Most protocols only allow broadcasting on a datagram-style socket.
		////////////////////////////////////////////////////////////////////////////////
		short Broadcast(void);									// Returns 0 if successfull, non-zero otherwise

		////////////////////////////////////////////////////////////////////////////////
		// Set socket to listen for connection requests
		//
		// Some protocols are limited to some maximum number of queued connections.
		// For instance, WinSock only allows for 5.  Requesting more than 5 queued
		// connections will cause this function to return an error.
		////////////////////////////////////////////////////////////////////////////////
		short Listen(												// Returns 0 if successfull, non-zero otherwise
			short sMaxQueued = 5);								// In:  Maximum number of queued connection requests


		////////////////////////////////////////////////////////////////////////////////
		// Accept request for connection.
		//
		// If this function fails, the specified client socket and address may have
		// been modified, but any such changes must not be relied upon!!!  What can be
		// relied upon is that the client socket will be in a "closed" state.
		////////////////////////////////////////////////////////////////////////////////
		short Accept(												// Returns 0 if successfull, non-zero otherwise
			RSocket* psocketClient,								// Out: Client socket returned here
			Address* paddressClient)							// Out: Client's address returned here (unless this is NULL)
			const;

		////////////////////////////////////////////////////////////////////////////////
		// Connect to address.
		//
		// If the RSocket::optDontBlock option was set on this socket, then this
		// function may return RSocket::errWouldBlock, which indicates that it is
		// still trying to connect, but has not yet managed to do so.  In that case,
		// this function should be called repeatedly (polled) until it returns either
		// an actual error message other than RSocket::errWouldBlock, which would
		// indicate that the connection attempt has failed, or 0, which indicates
		// that it has actually connected successfully.
		////////////////////////////////////////////////////////////////////////////////
		short Connect(												// Returns 0 if successfull, non-zero otherwise
			Address* paddress);									// In:  Remote address to connect to


		////////////////////////////////////////////////////////////////////////////////
		// Send data (only valid for connected sockets -- see also SendTo())
		//
		// For datagram (UDP) sockets, there is a one-to-one correspondence between
		// sends and receives.  Each send implies a matching receive.  The amount of
		// data sent must fit into a single block, whose size can be determined by
		// calling GetMaxDatagramSize().
		//
		// For stream (TCP) sockets, there is no direct correspondence between sends
		// and receive.  One send can be broken up and require multiple receives, and
		// and multiple sends can be coalesced into a single recieve.  There is no
		// limitation on the amount of data being sent.
		////////////////////////////////////////////////////////////////////////////////
		short Send(													// Return 0 if successfull, non-zero otherwise
			void* pBuf,												// In:  Pointer to data buffer
			long lNumBytes,										//	In:  Number of bytes to send
			long* plActualBytes);								// Out: Actual number of bytes sent


		////////////////////////////////////////////////////////////////////////////////
		// Send data to specified address.  For connected sockets, address is ignored.
		// See Send() for more information.
		////////////////////////////////////////////////////////////////////////////////
		short SendTo(												// Return 0 if successfull, non-zero otherwise
			void* pBuf,												// In:  Pointer to data buffer
			long lNumBytes,										//	In:  Number of bytes to send
			long* plActualBytes,									// Out: Actual number of bytes sent
			Address* paddress);									// In:  Address to send to


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
		short Receive(												// Returns 0 if successfull, non-zero otherwise
			void* pBuf,												// In:  Pointer to data buffer
			long lMaxBytes,										// In:  Maximum bytes that can fit in buffer
			long* plActualBytes);								// Out: Actual number of bytes received into buffer


		////////////////////////////////////////////////////////////////////////////////
		// Receive data and get source address.  See Receive() for more information.
		////////////////////////////////////////////////////////////////////////////////
		short ReceiveFrom(										// Returns 0 if successfull, non-zero otherwise
			void* pBuf,												// In:  Pointer to data buffer
			long lMaxBytes,										// In:  Maximum bytes that can fit in buffer
			long* plActualBytes,									// Out: Actual number of bytes received into buffer
			Address* paddress);									// Out: Source address returned here (unless this is NULL)


		////////////////////////////////////////////////////////////////////////////////
		// Status functions
		////////////////////////////////////////////////////////////////////////////////
		bool IsError(void);										// Returns true if there's an error (no error if not open)

		bool IsOpen(void)											// Returns true if socket is open
			{ return m_pProtocol ? true : false; }

		bool IsListening(void)									// Returns true if socket is listening
			{ return m_pProtocol ? m_pProtocol->m_bListening : false; }

		bool IsConnecting(void)									// Returns true if socket is trying to connect
			{ return m_pProtocol ? m_pProtocol->m_bConnecting : false; }

		bool IsConnected(void)									// Returns true if socket is connected
			{ return m_pProtocol ? m_pProtocol->m_bConnected: false; }

		bool CanAcceptWithoutBlocking(void);

		bool CanSendWithoutBlocking(void);

		bool CanReceiveWithoutBlocking(void);

		long CheckReceivableBytes(void);


		////////////////////////////////////////////////////////////////////////////////
		// Set callback function.  Setting callback to 0 disables the callback function.
		////////////////////////////////////////////////////////////////////////////////
		void SetCallback(											// Returns 0 if successfull, non-zero otherwise
			RSocket::BLOCK_CALLBACK callback)				// In:  New callback function (0 disables callback)
			{
			if (m_pProtocol)
				m_pProtocol->SetCallback(callback);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get callback function
		////////////////////////////////////////////////////////////////////////////////
		RSocket::BLOCK_CALLBACK GetCallback(void)			// Returns 0 if successfull, non-zero otherwise
			{
			if (m_pProtocol)
				return m_pProtocol->GetCallback();
			else
				return NULL;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Startup socket API.  This is normally called automatically when the first
		// RSocket is constructed, but can also be called "manually" so that other
		// static member functions may be called.
		////////////////////////////////////////////////////////////////////////////////
		static
		short Startup(												// Returns 0 if successfull, non-zero otherwise
			RSocket::ProtoType prototype,						// In:  Protocol type
			bool bAutoShutdown);									// In:  Whether to perform auto Shutdown()


		////////////////////////////////////////////////////////////////////////////////
		// Shutdown socket API.  This is normally called automatically when the last
		// RSocket is destroyed.  It is highly recommended that this NOT be called
		// manually, because any existing RSocket's will be invalidated.
		////////////////////////////////////////////////////////////////////////////////
		static
		void Shutdown(void);


		////////////////////////////////////////////////////////////////////////////////
		// Get maximum datagram size (applies to UDP only).  The minimum value is
		// supposed to be 512 bytes, but it is probably worth checking to be sure.
		// A value of 0 indicates that there is no limitation on size.
		////////////////////////////////////////////////////////////////////////////////
		static
		short GetMaxDatagramSize(								// Returns 0 if successfull, non-zero otherwise
			long* plSize);											// Out: Maximum datagram size (in bytes)


		////////////////////////////////////////////////////////////////////////////////
		// Get maximum number of sockets.  This may be a system "global" value, which
		// means that if other applications are using sockets, then the number available
		// to this application may be lower than the returned value.
		////////////////////////////////////////////////////////////////////////////////
		static
		short GetMaxSockets(										// Returns 0 if successfull, non-zero otherwise
			long* plNum);											// Out: Maximum number of sockets


		////////////////////////////////////////////////////////////////////////////////
		// Get address of specified host
		////////////////////////////////////////////////////////////////////////////////
		static
		short GetAddress(											// Returns 0 if successfull, non-zero otherwise
			char* pszName,											// In:  Host's name or dotted address (x.x.x.x)
			USHORT usPort,											// In:  Host's port number
			Address* paddress);									// Out: Address


		////////////////////////////////////////////////////////////////////////////////
		// Create broadcast address using specified port
		////////////////////////////////////////////////////////////////////////////////
		static void CreateBroadcastAddress(
			unsigned short usPort,								// In:  Port to broadcast to
			RSocket::Address* paddress);						// Out: Broadcast address returned here


		////////////////////////////////////////////////////////////////////////////////
		// Get the port of an existing (valid) address
		////////////////////////////////////////////////////////////////////////////////
		static
		unsigned short GetAddressPort(						// Returns port number
			Address* paddress);									// In:  Address to get port from


		////////////////////////////////////////////////////////////////////////////////
		// Set the port of an existing (valid) address
		////////////////////////////////////////////////////////////////////////////////
		static
		void SetAddressPort(
			USHORT usPort,											// In:  New port number
			Address* paddress);									// I/O: Address whose port is to be set


		////////////////////////////////////////////////////////////////////////////////
		// Get the name of the specified protocol.  This will always return a valid
		// pointer to a text string, even if the specified protocol is not valid.  In
		// such cases, the returned pointer will refer to an empty string.
		////////////////////////////////////////////////////////////////////////////////
		static
		const char* GetProtoName(								// Returns pointer to protocol's name
			ProtoType prototype)									// In:  Protocol type
			{
			if (prototype >= NumProtocols)
				prototype = NO_PROTOCOL;
			return ms_apszProtoNames[prototype];
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get the name of the current protocol
		////////////////////////////////////////////////////////////////////////////////
		static
		const char* GetProtoName(void)						// Returns pointer to protocol's name
			{
			return GetProtoName(ms_prototype);
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get the current protocol type
		////////////////////////////////////////////////////////////////////////////////
		static
		RSocket::ProtoType GetProtoType(void)
			{
			return ms_prototype;
			}

	protected:
		////////////////////////////////////////////////////////////////////////////////
		// Create specified protocol object
		////////////////////////////////////////////////////////////////////////////////
		static
		RProtocol* ConstructProtocol(							// Returns pointer to prototype if successfull, 0 otherwise
			ProtoType prototype);								// In:  Protocol type to create

};


#include "ProtoBSDIP.h"


// operator== cannot be defined until after all the protocol address types have been defined.
inline bool operator==(const RSocket::Address& lhs, const RSocket::Address& rhs)
	{
	switch (lhs.prototype)
		{
		case RSocket::TCPIP:
			return (*((const RProtocolBSDIP::AddressIP*)&lhs) == *((const RProtocolBSDIP::AddressIP*)&rhs));
			break;
		}
	TRACE("RSocket::Address::operator==(): Unknown protocol!\n");
	return false;
	}


#endif //SOCKET_H

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
