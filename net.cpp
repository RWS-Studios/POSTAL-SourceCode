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
// net.cpp
// Project: Nostril (aka Postal)
//
//	History:
//		08/18/97 MJR	Created this to hold some generic stuff.
//
//		08/27/97 MJR	Shortened text so it would fit in the text field.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "net.h"

#ifdef WIN32
	#define NETNAMESPACE
#else
	#define NETNAMESPACE		Net::
#endif

// Lookup tables associated with the CNetLimits::NetBandwidth enums.
#ifdef WIN32
namespace Net
	{
#endif

	long	NETNAMESPACE lBandwidthValues[Net::NumBandwidths] =
		{
		   1400,						// Analog14_4
		   2400,						// Analog28_8
		   2400,						// Analog33_6
		   2400,						// Analog57_6
		   5600,						// ISDN1Channel
		  11200,						// ISDN2Channel
		 800000,						// LAN10Mb
		1000000						// LAN100Mb
		};

	char* NETNAMESPACE BandwidthText[Net::NumBandwidths] =
		{
		"14.4 Modem",						// Analog14_4
		"28.8 Modem",						// Analog28_8
		"33.6 Modem",						// Analog33_6
		"57.6 Modem",						// Analog57_6
		"ISDN, 1 Channel",				// ISDN1Channel
		"ISDN, 2 Channels",				// ISDN2Channel
		"10Mb LAN (or T1)",				// LAN10Mb
		"100Mb LAN (or T3)"				// LAN100Mb
		};

#ifdef WIN32
	}
#endif


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
