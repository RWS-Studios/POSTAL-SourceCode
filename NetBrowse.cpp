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
// NetBrowse.cpp
// Project: RSPiX
//
// History:
//		09/01/97 MJR	Nearing the end of a major overhaul.
//
//		09/02/97 MJR	Changed so browse end now does the periodic broadcast
//							and hosts merely respond to them.  This saves bandwidth
//							on the host end, and in fact cuts down overall network
//							traffic because we'll only be generating these messages
//							when we're browsing, as opposed to having the hosts
//							constantly spew out messages, regardless of whether
//							anyone is listening.
//
//		09/06/97 MJR	Fixed so that it will properly drop hosts that no longer
//							exist.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "netbrowse.h"


////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////
CNetBrowse::CNetBrowse()
	{
	Reset();
	}


////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
CNetBrowse::~CNetBrowse()
	{
	Reset();
	}


////////////////////////////////////////////////////////////////////////////////
// Reset
////////////////////////////////////////////////////////////////////////////////
void CNetBrowse::Reset(void)
	{
	m_socketBrowse.Reset();
	m_lLastBroadcast = 0;
	m_usBasePort = 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup
////////////////////////////////////////////////////////////////////////////////
short CNetBrowse::Startup(							// Returns 0 if sucessfull, non-zero otherwise
	USHORT usPort,											// In:  Server's base port number
	RSocket::BLOCK_CALLBACK callback)				// In:  Blocking callback
	{
	short sResult = 0;

	// Make sure we start in a good state
	Reset();

	// Save base port
	m_usBasePort = usPort;

	// Create socket on which to broadcast
	sResult = m_socketBrowse.Open(m_usBasePort + Net::BroadcastPortOffset, RSocket::typDatagram, RSocket::optDontBlock, callback);
	if (sResult == 0)
		{
		// Set socket to broadcast mode
		sResult = m_socketBrowse.Broadcast();
		if (sResult == 0)
			{

			}
		else
			TRACE("CNetBrowse::StartBrowse(): Error putting socket into broadcast mode!\n");

		}
	else
		TRACE("CNetBrowse::StartBrowse(): Couldn't open broadcast socket!\n");

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown
////////////////////////////////////////////////////////////////////////////////
void CNetBrowse::Shutdown(void)
	{
	Reset();
	}


////////////////////////////////////////////////////////////////////////////////
// Update (must be called regularly!)
//
// The lists are updated, if necessary.  Note that only the phostsAll is
// important to this function, as it uses that list as the basis of its
// decisions to add or remove hosts.  This function will simply add to the
// other two lists as needed -- it does not care what they contain.  It is up
// to the caller to decide whether and when to clear those lists.
////////////////////////////////////////////////////////////////////////////////
void CNetBrowse::Update(
	Hosts* phostsAll,										// I/O:  List of all hosts
	Hosts* phostsAdded,									// I/O:  List of hosts that were added
	Hosts* phostsRemoved)								// I/O:  List of hosts that were removed
	{
	// Check if it's time to broadcast
	long lTime = rspGetMilliseconds();
	if ((lTime - m_lLastBroadcast) > Net::BroadcastInterval)
		{
		// Create message
		U8 buf1[4];
		buf1[0] = Net::BroadcastMagic0;
		buf1[1] = Net::BroadcastMagic1;
		buf1[2] = Net::BroadcastMagic2;
		buf1[3] = Net::BroadcastMagic3;

		// Create destination address (the address on which others will receive this message)
		RSocket::Address address;
		RSocket::CreateBroadcastAddress(m_usBasePort + Net::AntennaPortOffset, &address);

		// Broadcast the message
		long lBytesSent;
		short serr = m_socketBrowse.SendTo(buf1, sizeof(buf1), &lBytesSent, &address);
		if (serr == 0)
			{
			if (lBytesSent != sizeof(buf1))
				TRACE("CNetBrowse::Update(): Error sending broadcast (wrong size)!\n");
			}
		else
			{
			if (serr != RSocket::errWouldBlock)
				TRACE("CNetBrowse::Update(): Error sending broadcast!\n");
			}

		// If there was no error, reset the timer.  If there was an error, we want to
		// retry as soon as possible.  If the error is a recurring one that won't go
		// away, we'll be retrying every time this is called, but what the hell -- if
		// it isn't working, what are we gonna do instead?
		if (serr == 0)
			m_lLastBroadcast = lTime;
		}

	// Check for a reply to our broadcast.  If we get an incorrectly-sized message,
	// we simply ignore it -- this is a datagram socket, so if the message was larger
	// than we expected, the rest of it will be discarded, and if it was smaller, then
	// we can ignore it as well.  Bad messages could come from a foreign app that is
	// using the same port as us.  If we do get a message, the address of the sender
	// will be recorded -- this gives us the host's address!
	CHost host;
	long lReceived;
	U8 buf[sizeof(host.m_acName) + 4 + 4];
	short serr = m_socketBrowse.ReceiveFrom(buf, sizeof(buf), &lReceived, &host.m_address);
	if (serr == 0)
		{
		// Validate the message to make sure it was sent by another app of this
		// type, as opposed to some unknown app that happens to use the same port.
		if ((lReceived == sizeof(buf)) &&
			 (buf[0] == Net::BroadcastMagic0) &&
			 (buf[1] == Net::BroadcastMagic1) &&
			 (buf[2] == Net::BroadcastMagic2) &&
			 (buf[3] == Net::BroadcastMagic3))
			{
			// Copy the magic number.  The endian nature will always be correct because
			// the only entitity that is meant to recognize this value is the one
			// that sent it, so as long as the encoding and decoding of the bytes
			// is the same, that entity will get the same value that it sent.  All
			// other entities will see this as a meaningless value, which is fine.
			host.m_lMagic =
				((long)buf[4] & 0x000000ff) +
				(((long)buf[5] <<  8) & 0x0000ff00) +
				(((long)buf[6] << 16) & 0x00ff0000) +
				(((long)buf[7] << 24) & 0xff000000);

			// Copy the name
			strncpy(host.m_acName, (char*)&buf[8], sizeof(host.m_acName));
			host.m_acName[sizeof(host.m_acName)-1] = 0;

			// Init time we last heard from this host to "now"
			host.m_lLastHeardFrom = rspGetMilliseconds();

			// Change the host's port number from its antenna port to its base port
			unsigned short usHostBasePort = RSocket::GetAddressPort(&host.m_address) - Net::AntennaPortOffset;
			RSocket::SetAddressPort(usHostBasePort, &host.m_address);

			// Check if this host already exists in the list
			bool bExists = false;
			Hosts::Pointer p;
			for (p = phostsAll->GetHead(); p; p = phostsAll->GetNext(p))
				{
				if (host.IsSameHost(&phostsAll->GetData(p)))
					{
					// Update this host's time to "now"
					phostsAll->GetData(p).m_lLastHeardFrom = rspGetMilliseconds();
					
					// Set flag and stop
					bExists = true;
					break;
					}
				}

			// If host doesn't already exist, add it to the list
			if (!bExists)
				{
				phostsAll->InsertTail(host);
				phostsAdded->InsertTail(host);
				}
			}
		else
			TRACE("CNetBrowse::Update(): Validation failed -- another app may be sending crap to our port!\n");
		}
	else
		{
		if (serr != RSocket::errWouldBlock)
			TRACE("CNetBrowse::Update(): Error receiving broadcast!\n");
		}

	// Check for hosts that haven't been heard from in too long a time,
	// and should therefore be dropped.
	Hosts::Pointer p = phostsAll->GetHead();
	while (p)
		{
		Hosts::Pointer pNext = phostsAll->GetNext(p);
		if ((rspGetMilliseconds() - phostsAll->GetData(p).m_lLastHeardFrom) > Net::BroadcastDropTime)
			{
			// Drop this host by moving it from the "all" list to the "dropped" list
			phostsRemoved->InsertTail(phostsAll->GetData(p));
			phostsAll->Remove(p);
			}
		p = pNext;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Lookup host by name or hardwired address (like a TCP/IP dotted address).
// The specified port must be the host's "base port".
////////////////////////////////////////////////////////////////////////////////
// static
short CNetBrowse::LookupHost(						// Returns 0 if successfull, non-zero otherwise
	char* pszName,											// In:  Server's name or dotted address (x.x.x.x)
	USHORT usPort,											// In:  Server's port number
	RSocket::Address* paddress)						// Out: Addresss
	{
	// Try to get requested address 
	short sResult = RSocket::GetAddress(pszName, usPort, paddress);
	if (sResult != 0)
		TRACE("CNetBrowse::LookupHost(): Error getting host address!\n");
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
