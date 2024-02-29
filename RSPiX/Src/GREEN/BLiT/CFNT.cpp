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
// This is the file to deal with the CFNT, which is like
// the CAnim of text.  It is currently designed for FSPR1 images,
// but it could very easily be updated for TC use and any
// image type.
#include <string.h>

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
	#include "GREEN/BLiT/_BlitInt.H"
#else
	#include "BLIT.H"
	#include "_BlitInt.H" 
#endif

#include "Cfnt.h"

//========================================================
RFont::RFont()
	{
	m_pFontSets = NULL;
	m_sMaxCellHeight = m_sMaxCellWidth = m_sNumberOfScales = 0;
	}

void	RFont::EraseAll()
	{
	RFontSet *pfsn,*pfs = m_pFontSets;

	while (pfs)
		{
		pfsn = pfs->m_pNext;
		delete pfs;
		pfs = pfsn;
		}

	m_sMaxCellHeight = m_sMaxCellWidth = m_sNumberOfScales = 0;
	}

RFont::~RFont()
	{
	EraseAll();
	}
//========================================================
short RFont::Add(char* pszFileName)
	{
	RFile rfTemp;

	if (rfTemp.Open(pszFileName,"rb",RFile::LittleEndian/*,RFile::Ascii*/))
		{	
		TRACE("RFont::AddFont: Couldn't open %s\n",pszFileName);
		return -1;
		}

	Add(&rfTemp);
	rfTemp.Close();

	return 0;
	}

short RFont::Add(RFile* pcf)
	{

#ifdef _DEBUG

	if (!pcf)
		{
		TRACE("RFont::AddFont:null CNFile passed.\n");
		return -1;
		}

#endif

	char szCommand[256];

	// Read the basic header and verify
	pcf->Read(szCommand);
	if (strcmp(szCommand,"FONTFILE"))
		{
		TRACE("RFont::AddFont:Not a valid font file.\n");
		return -1;
		}

	short sVersion;

	pcf->Read(&sVersion);
	if (sVersion != 8)
		{
		TRACE("RFont::AddFont:Wrong version of font file!\n");
		return -1;
		}

	short sW,sH,sN;
	pcf->Read(&sH);
	pcf->Read(&sW);
	pcf->Read(&sN);

	TRACE("Loading Font, %hd by %hd, %hd scales\n",sW,sH,sN);

	short sDone = FALSE;
	while (!sDone)
		{	
		pcf->Read(szCommand);
		if (!strcmp(szCommand,"ENDFONT"))
			{
			sDone = TRUE;
			break;
			}

		if (!strcmp(szCommand,"FONTSET"))
			{
			short sH,sW;
			pcf->Read(&sH);
			pcf->Read(&sW);
			TRACE("FontSet: %hd by %hd\n",sW,sH);
			continue;
			}

		if (!strcmp(szCommand,"LETTER_"))
			{
			UCHAR ucASCII;
			RImage* pim = new RImage;

			pcf->Read(&ucASCII);
			pim->Load(pcf);
			AddLetter(pim,(short)ucASCII);
			long lBogus = pcf->Tell();
			}
		}

	return 0;
	}

// For FSPR1, you don't need the parameters
// Will return a +1 if overwriting an old letter!
//
short RFont::AddLetter(RImage* pimLetter,short sASCII,
							  short sKernL,short sKernR)
	{

#ifdef _DEBUG

	if (!pimLetter)
		{
		TRACE("RFont::AddLetter: null image!\n");
		return -1;
		}

#endif

	// First find if a font exists:
	RFontSet* pFont = m_pFontSets;
	RSpecialFSPR1*	pInfo = NULL;

	// try to handle a generic type efficiently:
	if (pimLetter->m_type == RImage::FSPR1)
		{
		pInfo = (RSpecialFSPR1*)pimLetter->m_pSpecialMem;

		if (!pInfo)
			{
			TRACE("RFont::AddLetter: Bad FSPR1 format!\n");
			return -1;
			}

		if (sASCII != -1) // FSPR1 case only
			{
			pInfo->m_u16ASCII = sASCII; // translate it...
			}
		}

	while (pFont)
		{
		if (pFont->m_sCellHeight == pimLetter->m_sHeight) break;
		pFont = pFont->m_pNext;
		}

	if (!pFont) // no match exists -> make a new font set..
		{
		pFont = new RFontSet;
		if (!pFont)
			{
			TRACE("RFont::AddLetter: Memory alloc error!\n");
			return -1;
			}
		pFont->m_sCellHeight = pimLetter->m_sHeight;
		pFont->m_sMaxWidth = pimLetter->m_sWidth;

		// Insert into the list so that it's smallest to largest:
		RFontSet* pInsertFont = m_pFontSets;
		m_sNumberOfScales++;

		if (!pInsertFont) m_pFontSets = pFont; // first one
		else
			{
			RFontSet* pLastFont = pInsertFont;

			while (pInsertFont)
				{
				if (pInsertFont->m_sCellHeight < pFont->m_sCellHeight)
					{
					pLastFont = pInsertFont; // save last position
					pInsertFont = pInsertFont->m_pNext; // keep looking

					if (pInsertFont) continue;
					// insert it at the end:
					pLastFont->m_pNext = pFont;
					}

				// insert before!
				pFont->m_pNext = pInsertFont; // stick before...
				if (pInsertFont == m_pFontSets) // insert head
					m_pFontSets = pFont;
				else // link to previous;
					pLastFont->m_pNext = pFont;
				break;
				}
			}
		}

	// Insert it into the font set:
	short sRet = 0;

	if (pFont->m_ppimCharacters[pInfo->m_u16ASCII])
		{
		//TRACE("RFont::AddLetter: WARNING! Overwriting old letter!\n");
		sRet = 1;
		delete pFont->m_ppimCharacters[pInfo->m_u16ASCII];
		}

	pFont->m_ppimCharacters[pInfo->m_u16ASCII] = pimLetter;
	//=====================================================
	// Update Font Set parameters
	//
	if (pimLetter->m_sWidth > pFont->m_sMaxWidth)
		pFont->m_sMaxWidth = pimLetter->m_sWidth;
	// (All fonts in the font set by DEFINITION have the same
	// height)

	//=====================================================
	// Update global font parameters:
	//
	if (pimLetter->m_sWidth > m_sMaxCellWidth)
		m_sMaxCellWidth = pimLetter->m_sWidth;

	if (pimLetter->m_sHeight > m_sMaxCellHeight)
		m_sMaxCellHeight = pimLetter->m_sHeight;

	return sRet;
	}

short RFont::Save(char* pszFileName)
	{
	RFile rfTemp;

	if (rfTemp.Open(pszFileName,"wb",RFile::LittleEndian/*,RFile::Ascii*/))
		{	
		TRACE("RFont::SaveFont: Couldn't open %s\n",pszFileName);
		return -1;
		}

	if (Save(&rfTemp)==0)
		{
		TRACE("RFont::SaveFont: %s saved!\n",pszFileName);
		}

	rfTemp.Close();

	return 0;
	}

// THIS is the font we are loading!
//
short RFont::Load(char* pszFileName)
	{
	RFile* pfileTemp = new RFile;

	if (pfileTemp->Open(pszFileName,"rb",RFile::LittleEndian/*,RFile::Ascii*/))
		{	
		TRACE("RFont::LoadFont: Couldn't open %s\n",pszFileName);
		delete pfileTemp;
		return -1;
		}

	if (Load(pfileTemp) == 0)
		{
		// TRACE("RFont::LoadFont: %s loaded!\n",pszFileName);
		pfileTemp->Close();
		delete pfileTemp;

		return 0;
		}

	pfileTemp->Close();
	delete pfileTemp;

	return -1;
	}

short RFont::Save(RFile* pcf)
	{

#ifdef _DEBUG
	if (!pcf) 
		{
		TRACE("RFont::SaveFont:Null Font passed!\n");
		return -1;
		}
#endif

	// Write the basic header
	pcf->Write("FONTFILE"); // type
	short sVersion = 8;
	pcf->Write(&sVersion);
	// Write overall font info..
	pcf->Write(&m_sMaxCellHeight);
	pcf->Write(&m_sMaxCellWidth);
	pcf->Write(&m_sNumberOfScales);
	// Save each font set:
	RFontSet* pfs = m_pFontSets;
	while (pfs)
		{
		pcf->Write("FONTSET");
		// Write overall font set info..
		pcf->Write(&pfs->m_sCellHeight);
		pcf->Write(&pfs->m_sMaxWidth);

		for (short i=0;i<256;i++)
			{
			if (pfs->m_ppimCharacters[i])
				{
				pcf->Write("LETTER_");
				pcf->Write((UCHAR*)&i);
				pfs->m_ppimCharacters[i]->Save(pcf);
				}
			}
		pfs = pfs->m_pNext;
		}

	pcf->Write("ENDFONT");

	return 0;
	}

// We are in a RFont, so it should exits...
//
short RFont::Load(RFile* pcf)
	{

#ifdef _DEBUG

	if (!pcf) 
		{
		TRACE("RFont::LoadFont:Null File passed!\n");
		return -1;
		}

#endif

	char string[20] = {0,};

	//------------------------------
	// Read the basic header
	//------------------------------
	pcf->Read(&string[0],9);
	if (strcmp(&string[0],"FONTFILE")) // not equal
		{
		TRACE("RFont::Load: Bad font file!\n");
		return -1;
		}; // type

	short sVersion;
	pcf->Read(&sVersion);

	if (sVersion != 8)
		{
		TRACE("RFont::Load: This version not supported\n");
		return -1;
		}

	//------------------------------
	// Write overall font info..
	//------------------------------

	pcf->Read(&m_sMaxCellHeight);
	pcf->Read(&m_sMaxCellWidth);
	pcf->Read(&m_sNumberOfScales);
	m_sNumberOfScales = 0; // This will be recreated!

	short sDummy = 0; // we don't need this info!

	// Load & Instantiate each font set by adding letters:
	while (!pcf->IsEOF())
		{
		pcf->Read(&string[0]);
		if (!strcmp(&string[0],"FONTSET")) // equal
			{
			// Don't need overall font set info..
			pcf->Read(&sDummy);
			pcf->Read(&sDummy);
			}

		else if (!strcmp(&string[0],"LETTER_")) // equal
			{
			UCHAR c;
			pcf->Read((UCHAR*)&c);
			RImage* pimLetter = new RImage;
			pimLetter->Load(pcf);
			if (pimLetter == NULL)
				{
				TRACE("RFont::Load: Bad Letter %c in file!\n",char(c));
				}
			else
				AddLetter(pimLetter); // create the font!
			}

		else if (!strcmp(&string[0],"ENDFONT")) break; // equal
		else
			{
			TRACE("RFont::Load: Bad font file!\n");
			return -1;
			}
		}

	return 0;
	}

// This selects the cached font closest in size to
// your request that is greater than or equal to the
// requested font height.  This does NOT guarantee that
// your character exists in a given font size.  A letter
// based font search may be implemented later.
//
// It returns null if no cached font was found larger than 
// the request size.
//
RFont::RFontSet* RFont::FindSize(short sCellH,double *pdScale)
	{
	RFontSet* pFont = NULL;
	RFontSet* pfntRet = NULL;
	*pdScale = 0.0;

	// Assumes fonts are stored largest first.
	if (m_pFontSets == NULL)
		{
		TRACE("RFont::FindSize: No fonts cached!\n");
		return NULL;
		}

	pFont = m_pFontSets;

	// Assume font's are ordered smallest to biggest:
	while ((pFont->m_sCellHeight < sCellH))
		{
		pFont = pFont->m_pNext;
		if (!pFont) break;
		}
	pfntRet = pFont;

	if (pfntRet) *pdScale = double(sCellH)/pfntRet->m_sCellHeight;
	return pfntRet;
	}

// This allows an editor to remove vertain FontSets out of a
// loaded font.  It will NOT allow you to delete the final
// fontset, which would leave a degenerate font.  You supply
// the fontset to remove.  It returns SUCCESS or FAILURE.
//
short	RFont::DeleteSet(RFontSet* pRemove)
	{
	RFontSet* pFont = m_pFontSets;
	// Must not degenerate the font:
	if (m_sNumberOfScales < 2) return FAILURE;

	// Find a match:
	short sMatch = FALSE;
	RFontSet* pPrevFont = NULL;

	while (pFont)
		{
		if (pFont == pRemove)
			{
			sMatch = TRUE;
			break;
			}

		pPrevFont = pFont;
		pFont = pFont->m_pNext;
		}

	if (!sMatch) return FAILURE;
	// Found a match:

	// Handle special cases:
	if (!pRemove->m_pNext)
		{
		// remove the tail
		pPrevFont->m_pNext = NULL;
		m_sMaxCellHeight = pPrevFont->m_sCellHeight;
		m_sMaxCellWidth = pPrevFont->m_sMaxWidth;
		}
	else if (!pPrevFont)
		{
		// remove the head
		m_pFontSets = pRemove->m_pNext;
		}
	else
		{
		// remove from center.
		pPrevFont->m_pNext = pRemove->m_pNext;
		}

	m_sNumberOfScales--;
	delete pRemove;

	return SUCCESS;
	}

//========================================================
RFont::RFontSet::RFontSet()
	{
	m_sMaxWidth = m_sCellHeight = 0;
	m_pNext = NULL;
	m_ppimCharacters = (RImage**) calloc(256,sizeof(RImage*));
	}

RFont::RFontSet::~RFontSet()
	{
	short i;
	for (i=0;i<256;i++) if(m_ppimCharacters[i]) 
		delete m_ppimCharacters[i];
	m_sMaxWidth = m_sCellHeight = 0;
	m_pNext = NULL;
	free(m_ppimCharacters);
	}
//========================================================
