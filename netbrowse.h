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
// netbrowse.h
// Project: RSPiX
//
//	History:
//		08/31/97 MJR	Started.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NETBROWSE_H
#define NETBROWSE_H

#include "RSPiX.h"
#include "socket.h"
#include "net.h"


////////////////////////////////////////////////////////////////////////////////
//
// CNetBrowse makes it easy to browse for hosts
//
////////////////////////////////////////////////////////////////////////////////
class CNetBrowse
	{
	//------------------------------------------------------------------------------
	// Classes
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// CHost contains basic info about a host
		////////////////////////////////////////////////////////////////////////////////
		class CHost
			{
			public:
				char					m_acName[Net::MaxHostNameSize];	// Name
				RSocket::Address	m_address;								// Address
				long					m_lMagic;								// Magic number
				long					m_lLastHeardFrom;						// Time we last heard from this host
				U32					m_u32User;								// User-definable value

			public:
				// Constructor
				CHost()
					{
					m_acName[0] = 0;
					m_address.Reset();
					m_lMagic = 0;
					m_lLastHeardFrom = 0;
					m_u32User = 0;
					}

				// Destructor
				~CHost()
					{
					}

				// operator=
				const CHost& operator=(const CHost& rhs)
					{
					strncpy(m_acName, rhs.m_acName, sizeof(m_acName));
					m_address = rhs.m_address;
					m_lMagic = rhs.m_lMagic;
					m_u32User = rhs.m_u32User;
					return *this;
					}

				// Compare two hosts
				bool IsSameHost(const CHost* rhs) const
					{
					// We ignore the user value in comparisons!!!!
					if ((strcmp(m_acName, rhs->m_acName) == 0) &&
						 (m_address == rhs->m_address) &&
						 (m_lMagic == rhs->m_lMagic))
						return true;
					return false;
					}
			};

	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:
		// List of hosts
		typedef RFList<CHost> Hosts;

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		RSocket			m_socketBrowse;						// Socket used to browse for hosts
		long				m_lLastBroadcast;						// Last broadcast time
		unsigned short	m_usBasePort;							// Base port

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CNetBrowse();


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CNetBrowse();


		////////////////////////////////////////////////////////////////////////////////
		// Reset
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void);


		////////////////////////////////////////////////////////////////////////////////
		// Startup
		////////////////////////////////////////////////////////////////////////////////
		short Startup(												// Returns 0 if sucessfull, non-zero otherwise
			USHORT usPort,											// In:  Server's base port number
			RSocket::BLOCK_CALLBACK callback);				// In:  Blocking callback


		////////////////////////////////////////////////////////////////////////////////
		// Shutdown
		////////////////////////////////////////////////////////////////////////////////
		void Shutdown(void);


		////////////////////////////////////////////////////////////////////////////////
		// Update (must be called regularly!)
		//
		// The lists are updated, if necessary.  Note that only the phostsAll is
		// important to this function, as it uses that list as the basis of its
		// decisions to add or remove hosts.  This function will simply add to the
		// other two lists as needed -- it does not care what they contain.  It is up
		// to the caller to decide whether and when to clear those lists.
		////////////////////////////////////////////////////////////////////////////////
		void Update(
			Hosts* phostsAll,										// I/O:  List of all hosts
			Hosts* phostsAdded,									// I/O:  List of hosts that were added
			Hosts* phostsRemoved);								// I/O:  List of hosts that were removed


		////////////////////////////////////////////////////////////////////////////////
		// Lookup host by name or hardwired address (like a TCP/IP dotted address).
		// The specified port must be the host's "base port".
		////////////////////////////////////////////////////////////////////////////////
		static
		short LookupHost(											// Returns 0 if successfull, non-zero otherwise
			char* pszName,											// In:  Server's name or dotted address (x.x.x.x)
			USHORT usPort,											// In:  Server's port number
			RSocket::Address* paddress);						// Out: Addresss
	};


#endif //NETBROWSE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
