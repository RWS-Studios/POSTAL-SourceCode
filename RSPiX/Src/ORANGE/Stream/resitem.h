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
#ifndef RESITEM_H
#define RESITEM_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
					  
//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
// Forward define class.
class	CRes;

class CResItem
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CResItem();
		// Constructura Especial! Ole!
		CResItem(char* pszName, uint8_t* puc, int32_t lSize, CRes* pRes);

		// Destructor.
		~CResItem();

	public:	// Methods.
		// Lock this item b4 using.
		// Returns new reference count.
		int16_t Lock(void);
		// Unlock this item when done.  Deletes object if m_sRefCnt hits 0.
		// Returns new reference count.
		int16_t Unlock(void);

	public:	// Members.
		char*		m_pszName;		// Resource name.
		uint8_t*	m_puc;			// Resource data.
		int32_t		m_lSize;			// Resource size.
		int16_t		m_sRefCnt;		// Number of items using this resource.
		CRes*		m_pRes;			// Res Manager that owns this resource.
	};

typedef CResItem* PRESITEM;

#endif // RESITEM_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
