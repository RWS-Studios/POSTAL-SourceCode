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
////////////////////////////////////////////////////////////////////////////////
//
// channel.cpp
// Project: RSPiX
//
// This module impliments the channel-related classes.
//
// History:
//		02/01/97 MJR	Started.
//
//		06/27/97	JMI	#if 0'ed out RChannelSet which does not specify the 
//							template arguments to its base class RChannel.
//
//		07/13/97 MJR	Moved RChannelSet stuff out to a separate file that isn't
//							even part of RSPiX yet.
//							Moved a bunch of stuff from channel.h into here.
//
////////////////////////////////////////////////////////////////////////////////

#include "channel.h"


// Usefull test code to make sure it will really compile.  If templated stuff
// isn't actually instantiated, the compiler will miss many (or all) errors.
RChannel<short> test;
RChannel<long> test2(RChannel_ArrayOfPtrs);


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
