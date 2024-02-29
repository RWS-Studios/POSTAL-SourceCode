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
#ifndef PIXEL_H
#define PIXEL_H


class CPixel
	{
	public:
		CPixel()
			{ Init(); }
		CPixel(short sBitsPerPixel)
			{ ASSERT(sBitsPerPixel % 8 == 0); m_sSize = sBitsPerPixel / 8; Init(); }

		void Init(void)
			{ for (short i = 0; i < sizeof(m_au8); i++) m_au8[i] = 0; }

		U8 GetU8Val(void)
			{ return m_au8[0]; }
		U16 GetU16Val(void)
			{ return *((U16*)m_au8); }
		U32 GetU24Val(void)
			{ return *((U32*)m_au8); }
		U32 GetU32Val(void)
			{ return *((U32*)m_au8); }

		void SetVal(U8 val)
			{ memcpy(m_au8, &val, sizeof(val)); }
		void SetVal(U16 val)
			{ memcpy(m_au8, &val, sizeof(val)); }
		void SetVal24(U32 val)
			{ memcpy(m_au8, &val, 3); }
		void SetVal(U32 val)
			{ memcpy(m_au8, &val, sizeof(val)); }

		void SetVal(UCHAR* puc)
			{ memcpy(m_au8, puc, m_sSize); }

		int operator <(CPixel &pixel)
			{ return (GetU32Val() < pixel.GetU32Val()); }

		int operator >(CPixel &pixel)
			{ return (GetU32Val() > pixel.GetU32Val()); }
		
		int operator ==(CPixel &pixel)
			{
			if (memcmp(m_au8, pixel.m_au8, MIN(m_sSize, pixel.m_sSize)) == 0)
				return TRUE;
			else
				return FALSE;
			}

	public:
		U8		m_au8[sizeof(U32)];
		short	m_sSize;
	};

#endif	// PIXEL_H
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
