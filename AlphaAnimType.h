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
// AlphaAnimType.h
// Project: Postal
//
////////////////////////////////////////////////////////////////////////////////

#ifndef ALPHAANIMTYPE_H
#define ALPHAANIMTYPE_H

// Simple wrapper class for each frame of alpha animation
class CAlphaAnim
	{
	public:
		short m_sNumAlphas;										// Number of alpha images (could be 0!)
		short m_sX;													// Offset from hotspot to upper-left corner of image
		short m_sY;													// Offset from hotspot to upper-left corner of image
		RImage m_imColor;											// "Normal" 8-bit color image
		RImage* m_pimAlphaArray;								// Array of alpha image's (could be empty!)

	public:
		CAlphaAnim()
			{
			m_pimAlphaArray = 0;
			Reset();
			};
		
		~CAlphaAnim()
			{
			Reset();
			};

		CAlphaAnim& operator=(const CAlphaAnim& rhs)
			{
			Reset();
			m_sNumAlphas = rhs.m_sNumAlphas;
			m_sX = rhs.m_sX;
			m_sY = rhs.m_sY;
			m_imColor = rhs.m_imColor;
			Alloc(m_sNumAlphas);
			rspObjCpy(m_pimAlphaArray, rhs.m_pimAlphaArray, m_sNumAlphas);
			return *this;
			}

		bool operator==(const CAlphaAnim& rhs) const
			{
			// Comparing two of these objects is a major undertaking.  Instead,
			// we'll always say that they are different.  This is not a great
			// solution.  In fact, it sucks.  But what the hell...
			return false;
			}

		void Reset(void)
			{
			Free();
			m_sNumAlphas = 0;
			m_sX = 0;
			m_sY = 0;
			}

		void Alloc(short sNumAlphas)
			{
			Free();
			if (sNumAlphas > 0)
				{
				m_pimAlphaArray = new RImage[sNumAlphas];
				ASSERT(m_pimAlphaArray != 0);
				}
			m_sNumAlphas = sNumAlphas;
			}

		void Free(void)
			{
			delete []m_pimAlphaArray;
			m_pimAlphaArray = 0;
			}

		short Load(RFile* pFile)
			{
			pFile->Read(&m_sNumAlphas);
			pFile->Read(&m_sX);
			pFile->Read(&m_sY);
			m_imColor.Load(pFile);
			Alloc(m_sNumAlphas);
			for (short s = 0; s < m_sNumAlphas; s++)
				m_pimAlphaArray[s].Load(pFile);
			return pFile->Error();
			}

		short Save(RFile* pFile)
			{
			pFile->Write(m_sNumAlphas);
			pFile->Write(m_sX);
			pFile->Write(m_sY);
			m_imColor.Save(pFile);
			for (short s = 0; s < m_sNumAlphas; s++)
				m_pimAlphaArray[s].Save(pFile);
			return pFile->Error();
			}
	};

#endif //ALPHAANIMTYPE_H

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
