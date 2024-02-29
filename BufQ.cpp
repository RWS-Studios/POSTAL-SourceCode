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
// BufQ.CPP
// 
// History:
//		05/24/97 JMI	Started.
//
//		05/24/97	JMI	Added UnGet().
//
//		08/14/97 MJR	Moved everything into this header so it would inline.
//
//////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "BufQ.h"

// Nothing to do here.  It's all in the header for maximum inlining!


// This was for testing the code efficiency
#if 0
void test(void);
void test(void)
	{
	CBufQ buf;
	U8 val = 0;
	buf.Get(&val);
	buf.Put(val);
	}
#endif
	
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
