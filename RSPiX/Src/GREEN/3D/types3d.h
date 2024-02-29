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
// types3d.h
// Project: RSPiX\Green\3d
//
// This file stores the "high level" data types (containers) needed by the
// renderer.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef TYPES3D_H
#define TYPES3D_H

#include "Blue.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/File/file.h"
	#include "ORANGE/color/colormatch.h"
	#include "ORANGE/QuickMath/VectorMath.h"
#else
	#include "file.h"
	#include "colormatch.h"
	#include "vectormath.h"
#endif


////////////////////////////////////////////////////////////////////////////////
// This is currently a flat texture used with small 
// polygons, i.e., 1 color per triangle.
// It may be mapped or unmappable.
//
// Note that the engine only uses the index colors.
////////////////////////////////////////////////////////////////////////////////
class RTexture
	{
	//------------------------------------------------------------------------------
	// Types
	//------------------------------------------------------------------------------
	public:
		// These values are used as bit masks, so you can indicate it
		// has one thing, the other, neither, or both.
		enum
			{
			HasIndices = 1,
			HasColors = 2
			} HasFlags;

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	public:
		short m_sNum;												// Number of colors in array(s)
		UCHAR* m_pIndices;										// Array of indices
		RPixel32* m_pColors;										// Array of colors

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		// Default constructor
		RTexture()
			{
			Init();
			}

		// Copy constructor
		RTexture(const RTexture& other)
			{
			Init();
			*this = other;
			}

		// Alternate constructor
		RTexture(short sNum)
			{
			Init();
			Alloc(sNum);
			}

		// Destructor
		~RTexture()
			{
			Free();
			}

		// Assignment operator
		RTexture& operator=(const RTexture& rhs)
			{
			Free();

			m_sNum = rhs.m_sNum;

			if (rhs.m_pIndices != NULL)
				{
				AllocIndices();
				rspObjCpy(m_pIndices, rhs.m_pIndices, (size_t)m_sNum);
				}

			if (rhs.m_pColors != NULL)
				{
				AllocColors();
				rspObjCpy(m_pColors, rhs.m_pColors, (size_t)m_sNum);
				}
			
			return *this;
			}

		// Overloaded == (equality) operator
		bool operator==(const RTexture& rhs) const
			{
			bool result = true;
			if (m_sNum == rhs.m_sNum)
				{
				if (m_sNum > 0)
					{
					// If both pointers are non-zero then we compare their data
					if (m_pIndices && rhs.m_pIndices)
						result = rspObjCmp(m_pIndices, rhs.m_pIndices, (size_t)m_sNum);
					else
						{
						// If both pointers are not NULL then they obviously don't match
						if ( !((m_pIndices == NULL) && (rhs.m_pIndices == NULL)) )
							result = false;
						}

					if (result == true)
						{
						// If both pointers are non-zero then we compare their data
						if (m_pColors && rhs.m_pColors)
							result = rspObjCmp(m_pColors, rhs.m_pColors, (size_t)m_sNum);
						else
							{
							// If both pointers are not NULL then they obviously don't match
							if ( !((m_pColors == NULL) && (rhs.m_pColors == NULL)) )
								result = false;
							}
						}
					}
				}
			else
				result = false;
			return result;
			}

		// Allocate specified number of indices and colors
		void Alloc(short sNum);

		// Allocate same number of indices as current number of colors
		void AllocIndices(void);

		// Allocate same number of colors as current number of indices
		void AllocColors(void);

		// Free indices and colors
		void Free(void);

		// Free indices only
		void FreeIndices(void);

		// Free colors only
		void FreeColors(void);

		// Load from file
		short	Load(RFile* fp);

		// Save to file
		short	Save(RFile* fp);

		// Map colors onto the specified palette.  For each color, the best
		// matching color is found in the  palette, and the associated palette
		// index is written to the array of indices.  If the array of indices
		// doesn't exist, it will be created.
		void Remap(
			short sStartIndex,
			short sNumIndex,
			UCHAR* pr,
			UCHAR* pg,
			UCHAR* pb,
			long linc);

		// Unmap colors from the specified palette and put them into the colors
		// array.  If the array of colors doesn't exist, it will be created.
		void 
		Unmap(
			UCHAR* pr,
			UCHAR* pg,
			UCHAR* pb,
			long lInc)
			;

		// Muddy or brighten or darken.  Applies the specified brightness value
		// to every nth color (where n == lInc).
		void
		Adjust(
			float fAdjustment,	// In:  Adjustment factor (1.0 == same, < 1 == dimmer, > 1 == brighter).
			long lInc)				// In:  Number of colors to skip.
			;

	private:
		// Init
		void Init(void)
			{
			m_sNum = 0;
			m_pIndices = NULL;
			m_pColors = NULL;
			}
	};


////////////////////////////////////////////////////////////////////////////////
// A mesh is basically an array of triangles, where each triangle consists of 3
// indices that refer to RP3d's stored in a separate CPointArray3d object.
////////////////////////////////////////////////////////////////////////////////
class RMesh
	{
	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	public:
		short m_sNum;												// Number of triangles in array (3 elements per triangle!)
		U16* m_pArray;												// Array of indices (3 per triangle!)

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		// Default constructor
		RMesh(void)
			{
			Init();
			}

		// Copy constructor
		RMesh(const RMesh& other)
			{
			Init();
			*this = other;
			}

		// Alternate constructor
		RMesh(short sNum)
			{
			Init();
			Alloc(sNum);
			}

		// Destructor
		~RMesh()
			{
			Free();
			}

		// Assignment operator
		RMesh& operator=(const RMesh& rhs)
			{
			Free();

			m_sNum = rhs.m_sNum;

			if (rhs.m_pArray != NULL)
				{
				Alloc(rhs.m_sNum);
				rspObjCpy(m_pArray, rhs.m_pArray, (size_t)(m_sNum * 3));
				}

			return *this;
			}

		// Overloaded == (equality) operator
		bool operator==(const RMesh& rhs) const
			{
			bool result = true;
			if (m_sNum == rhs.m_sNum)
				{
				if (m_sNum > 0)
					{
					// If both pointers are non-zero then we compare their data
					if (m_pArray && rhs.m_pArray)
						result = rspObjCmp(m_pArray, rhs.m_pArray,  (size_t)(m_sNum * 3));
					else
						{
						// If both pointers are not NULL then they obviously don't match
						if ( !((m_pArray == NULL) && (rhs.m_pArray == NULL)) )
							result = false;
						}
					}
				}
			else
				result = false;
			return result;
			}

		// Allocate specified number of triangles
		void Alloc(short sNum);

		// Free triangles
		void Free(void);

		// Load from file
		short	Load(RFile* fp);
		
		// Save to file
		short	Save(RFile* fp);

	protected:
		// Init
		void Init(void)
			{
			m_sNum = 0;
			m_pArray = NULL;
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
class RSop
	{
	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	public:
		long m_lNum;												// Number of points in array (only 65536 currently accessible)
		RP3d*	m_pArray;											// Array of points

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		// Default constructor
		RSop()
			{
			Init();
			}

		// Copy constructor
		RSop(const RSop& other)
			{
			Init();
			*this = other;
			}

		// Alternate constructor
		RSop(long lNum)
			{
			Init();
			Alloc(lNum);
			}

		// Destructor
		~RSop()
			{
			Free();
			}

		// Assignment operator
		RSop& operator=(const RSop& rhs)
			{
			Free();

			m_lNum = rhs.m_lNum;

			if (rhs.m_pArray != NULL)
				{
				Alloc(rhs.m_lNum);
				rspObjCpy(m_pArray, rhs.m_pArray, (size_t)m_lNum);
				}

			return *this;
			}

		// Overloaded == (equality) operator
		bool operator==(const RSop& rhs) const
			{
			bool result = true;
			if (m_lNum == rhs.m_lNum)
				{
				if (m_lNum > 0)
					{
					// If both pointers are non-zero then we compare their data
					if (m_pArray && rhs.m_pArray)
						result = rspObjCmp(m_pArray, rhs.m_pArray, (size_t)m_lNum);
					else
						{
						// If both pointers are not NULL then they obviously don't match
						if ( !((m_pArray == NULL) && (rhs.m_pArray == NULL)) )
							result = false;
						}
					}
				}
			else
				result = false;
			return result;
			}

		// Allocate specified number of points
		void Alloc(long lNum);
		
		// Free points
		void Free(void);

		// Load from file
		short	Load(RFile* fp);
		
		// Save to file
		short	Save(RFile* fp);

	protected:
		// Init
		void Init(void)
			{
			m_lNum = 0;
			m_pArray = NULL;
			}
	};


#endif // TYPES3D_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
