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
// yatime.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the CTime class, which manages game time.
//
// History:
//		01/22/97 MJR	Started.
//
//		07/14/97 MJR	Renamed module to yatime.cpp/.h to avoid conflicts with
//							<time.h>, which are only conflicts because the VC++
//							compiler doesn't properly differentiate between #include's
//							using <> and "".
//
////////////////////////////////////////////////////////////////////////////////
#define YATIME_CPP

#include "yatime.h"

// CTime is currently implimented entirely in the header file.  This file
// exists in case any of that gets big enough that we don't want it inline'd
// anymore!


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
