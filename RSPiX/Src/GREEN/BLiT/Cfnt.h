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
#ifndef CFNT_H
#define CFNT_H
//====================
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
	#include "GREEN/BLiT/_BlitInt.H"
#else
	#include "BLIT.H"
	#include "_BlitInt.H" 
#endif 
//====================

// This class is used to hold the information for
// a font.  Actual printing
// is done at a higher level by RPrint.
// It will support kerning information.
// Cell dimensions are in pixels.
// Fonts are stored in a singly linked list, largest first.

class RFont
	{
public:
	//---------------
	RFont();
	~RFont();
	void	EraseAll();
	//---------------
	//---------------
	class RFontSet
		{
	public:
		//---------------
		RFontSet();
		~RFontSet(); // free associated images...
		//---------------

		RImage** m_ppimCharacters; // FSPR1 has all kerning info inside
		short	m_sCellHeight;
		short m_sMaxWidth;
		RFontSet* m_pNext;
		};
	//--------------- USER STUFF
	short Save(char* pszFileName);
	short Save(RFile* pcf);
	short Load(char* pszFileName);
	short Load(RFile* pcf);
	//--------------- UTILITY STUFF

	short Add(char* pszFileName);
	short Add(RFile* pcf);
	short AddLetter(RImage* pimLetter, // if FSPR1, don't need other arguements
		short sASCII=-1,short sKernL=0,short sKernR=0);
	// pdScale will be <= 1.0
	RFontSet* FindSize(short sCellH,double *pdScale);

	short	DeleteSet(RFontSet* pRemove); // will NOT delete the last FontSet!
	//---------------
	short m_sMaxCellHeight;
	short m_sMaxCellWidth;
	short m_sNumberOfScales;
	RFontSet* m_pFontSets;
	};


//====================
#endif
