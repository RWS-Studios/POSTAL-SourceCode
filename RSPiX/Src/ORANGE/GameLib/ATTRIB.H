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
//*****************************************************************************
//
// ATTRIB.H
//
//*****************************************************************************

#ifndef attrib_h
#define attrib_h

#define ATTRIBUTE_MASK	0xffc0
#define CONTOUR_MASK		0x006f

typedef struct tag_CONTOUR
{
 	WORD block[16];
} CONTOUR;

typedef struct tag_ATTRIB
{
	short s;
} ATTRIB;


class CAttribute
{
protected:

	WORD*    m_pAttribMap;			//Pointer to attribute map
	CONTOUR* m_pAttribCont;
	short		m_sMapWidth;

public:
	CAttribute();
	~CAttribute();

	void Reset();
	void FromBlock();
	void ToBlock();
	void PutBlock();
	void GetReward();
	BOOL GetPointAttrib(short sX, short sY, WORD* wAttribute);
	short GetPointSurface(short sX, short sY);

};

#endif //attrib_h

//*****************************************************************************
// EOF
//*****************************************************************************

