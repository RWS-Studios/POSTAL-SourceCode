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
//THIS FILE IS ARCHAIC!  REMOVE IT FROM YOUR PROJECT AND
//ADD FILE RPrint.cpp

//***************  THIS SHOULD BE CALLED FNT.CPP  ***********************

#include <string.h>

// This is the higher level printing control.
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
#else
	#include "BLIT.H"
#endif

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/_BlitInt.H"
#else
	#include "_BlitInt.H" 
#endif

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/_ic.h"
#else
	#include "_ic.h" 
#endif

// Turn off warnings for assignments within expressions
#ifdef __MWERKS__
	#pragma warn_possunwant off
#endif

//*************************************************************
// Actual printing and font control code!
//*************************************************************
//**************  RFontOld... a management system  ************
//*************************************************************

//*************************************************************
RFontOld::RFontOld()
	{
	m_pszFontName = NULL;
	m_sMaxCellHeight = 0;
	m_pHead = m_pFontList = NULL;
	}

RFontOld::RFontOld(char*	pszName)
	{
	m_pszFontName = (char*)
		calloc(1,strlen(pszName)+1);

	strcpy(m_pszFontName,pszName);
	m_sMaxCellHeight = 0;
	m_pHead = m_pFontList = NULL;
	}

RFontOld::~RFontOld()
	{
	if (m_pszFontName) free(m_pszFontName);
	if (m_pFontList)
		{
		FontNode* ftemp;
		RImage* pimTemp;
		short i;
		while (m_pFontList) 
			{
			// Free each CharNode:
			for (i=0;i<256;i++)
				{
				if (pimTemp = m_pFontList->pLetters[i]) 
					delete m_pFontList->pLetters[i];	
				}
			// Free each FontSize
			ftemp = m_pFontList->pNext;
			free(m_pFontList);
			m_pFontList = ftemp;
			}
		}
	}

//=========  scaling 

void	RFontOld::AddLetter(RImage* pimLetter)
	{

#ifdef _DEBUG

	if (pimLetter == NULL)
		{
		TRACE("AddLetter: Null Image passed!\n");
		return;
		}

	if (pimLetter->m_type != RImage::FSPR1)
		{
		TRACE("AddLetter: Convert Image to ype FSPR1 first!\n");
		return;
		}

#endif

	// determine if chosen font size exists:
	FontNode* fCurSize = NULL;
	FontNode* fTemp;
	RSpecialFSPR1* pInfo = (RSpecialFSPR1*)pimLetter->m_pSpecial;

	if (fTemp = m_pFontList) // List of cached sizes:
		while (fTemp)
			{
			if (fTemp->sCellHeight == pimLetter->m_sHeight)
				{fCurSize = fTemp;fTemp = NULL;}
			else
				fTemp = fTemp->pNext;
			}
	// if no size fits, start a new cached size for this font:
	if (fCurSize == NULL)
		{							// so char ptrs are NULL:
		fTemp = (FontNode*) calloc(1,sizeof(FontNode));
		fTemp->sCellHeight = pimLetter->m_sHeight;
		fTemp->sUnproportionalWidth = pimLetter->m_sWidth;
		// Install this ACTUAL letter:
		fTemp->pLetters[pInfo->m_u16ASCII] = pimLetter;
		
		// Update the entire font:
		m_sMaxCellHeight = MAX(m_sMaxCellHeight,
			pimLetter->m_sHeight);

		// install the new font size
		if (m_pHead)
			{
			m_pHead->pNext = fTemp;
			m_pHead = fTemp;
			}
		else	// first char in this font size:
			{
			m_pHead = m_pFontList = fTemp;
			}

		}
	else
		{
		// install into an existing font size, fCurSize:
		fCurSize->sUnproportionalWidth = 
			MAX(pimLetter->m_sWidth,
			fCurSize->sUnproportionalWidth);

		if (fCurSize->pLetters[pInfo->m_u16ASCII])
			{
			TRACE("Addletter: warning, overwriting an existing letter\n");
			delete fCurSize->pLetters[pInfo->m_u16ASCII];
			}
		fCurSize->pLetters[pInfo->m_u16ASCII] = pimLetter;
		}
	}

// Create a font from a set of hex named 8-bit BMPs
//
/*
short RFontOld::Create(char *pszFontName,char *pszFullInputName,
						 UCHAR ucInitialTrim,UCHAR ucBackCol,short sTrimTop,
						short sMaxH)
	{
	// Create a temp FNT, and a base CImage:
	RFontOld fTemp;
	CImage* pimLetter = NULL;
	FILE* fp = NULL;
	char	szName[256];
	char* pszName = (char*)calloc(1,256);

	// ***** debugging only!
	CImage *pimBuf, *pimScreen;
	rspNameBuffers(&pimBuf,&pimScreen);
	fTemp.m_sMaxCellHeight = 0;

	// Create Letters:
	for (short i=0;i<256;i++)
		{
		sprintf(szName,pszFullInputName,i);
		fp = fopen(szName,"r");
		if (fp == NULL) continue;
		fclose(fp);
		TRACE("Character %02x (%03hd) found!\n",i,i);
		pimLetter = new CImage();

		if (pimLetter->LoadDib(szName) != SUCCESS)
			{
			TRACE("-- Error Loading bmp file %s!\n",szName);
			continue;
			}

		// ***** debugging only!
		rspBlit(pimLetter,pimScreen,0,0,0,0,short(pimLetter->lWidth),
			short(pimLetter->lHeight) );

		// Add it in:
		//rspSetConvertToFSPR1((ULONG)ucInitialTrim,(ULONG)ucBackCol,sTrimTop,sMaxH,(UCHAR)i);
		//short sX = 0,sY = 0,sW = pimLetter
		// Need to use rsplasso with this...
		// Will be writing a utility to do this...
		pimLetter->Convert(FSPR1);
		fTemp.AddLetter(pimLetter);
		fTemp.m_sMaxCellHeight = MAX(fTemp.m_sMaxCellHeight,(short)pimLetter->lHeight);

		}
	strcpy(pszName,pszFontName);
	fTemp.m_pszFontName = pszName;
	// Save it!
	// Work on this a bit!
	fTemp.Save(""); // local path

	return 0;
	}
*/

// Open my own CNFile
short RFontOld::Save(char* pszFile)
	{
	RFile  File;

	if (File.Open(pszFile,"wb",RFile::LittleEndian))
		{
		TRACE("RFontOld::Save:  Can't open %s\n",pszFile);
		}

	Save(&File);


	File.Close();
	return SUCCESS;
	}

// Give it an existing file
//
short	RFontOld::Save(RFile* /*pFile*/)
	{
	/*
	//char	fname[255];
	
	// using fprintf/fscanf with \n... hope it all works...
	// for now, assume valid input:
	
	// create output file name:
	//sprintf(fname,"%s/%s.fnt",pszPath,m_pszFontName);
	//TRACE("Saving font %s\n",fname);
	//FILE*	fp = fopen(fname,"wb");

	if (!pFile) return FAILURE;

	// Overall header:
	fprintf(fp,"FONTFIL2\n");	// type id
	fprintf(fp,"%s\n",m_pszFontName);
	fprintf(fp,"%hd\n",m_sMaxCellHeight); // for user's conveniencs...

	// This is merely a collection of letters.
	// It makes sense for them to be ordered by height, but it
	// doesn't really matter as AddLetter automatically detects
	// new font sizes...

	// We do not, currently, support the kerning matrix...
	FontNode*	pCurFont = m_pFontList;
	CImage*	pimCurLetter;
	short	i;

	while	(pCurFont)	// loop through cached sizes:
		{
		// Describe this font:
		fprintf(fp,"F%hd\n%hd\n",pCurFont->sCellHeight,pCurFont->sUnproportionalWidth);

		// enumerate each letter:
		for (i=0;i<256;i++)
			{
			if (pimCurLetter = pCurFont->pLetters[i])
				{
				CSpecialFSPR1* pInfo = (CSpecialFSPR1*)pimCurLetter->pSpecial;

				// save this letter:
				fprintf(fp,"L%hd\n%ld\n%ld\n%hd\n%hd\n%hd\n%hd\n%hd\n",
					pInfo->m_u16ASCII,pimCurLetter->lWidth,pimCurLetter->lHeight,
					(short)0, (short)0,(short)0,(short)0,(short)0); // padding
					/* NYI:
					pimCurLetter->sBaseH,
					pimCurLetter->sOffsetL,pimCurLetter->sOffsetT,pimCurLetter->sOffsetW,
					pimCurLetter->sOffsetH);

				// NOTE: we don't currently deal with the kerning list!

				// Save the current letter!
			
				UCHAR	ucCheckSum;
				UCHAR*	pCode = pInfo->m_pCode;
				ucCheckSum = (UCHAR)0; 
				for (long i=0;i < (long)pInfo->m_lSize;i++,pCode++)
					ucCheckSum ^= *pCode;

				fprintf(fp,"%02x\n",(unsigned short) ucCheckSum);
				fprintf(fp,"%ld\n",pInfo->m_lSize);
				for (i=0,pCode = pInfo->m_pCode;i < (long)pInfo->m_lSize;
							i++,pCode++)
					fprintf(fp,"%c",*pCode);
				fprintf(fp,"\n");
				}
			}

		pCurFont = pCurFont->pNext;

		}

	fclose(fp);
	*/
	return SUCCESS;
	}

// Backwards?
short	RFontOld::Load(char*	pszPath,char*	pszName)
	{
	char	fname[255];

#ifdef WIN32
	sprintf(fname,"%s/%s.fnt",pszPath,pszName);
#else // for MAC
	sprintf(fname,"%s:%s.fnt",pszPath,pszName);
#endif

	return this->Load(fname);
	}

// Must supply correctly formatted path AND etension.
//
short	RFontOld::Load(char*	pszName)
	{
	char	pszCompare[255];
	char	c;
	short	sDone = FALSE;
	short	i,s1;//,s2,s3,s4;
	long l1;
	
	TRACE("Loading font %s\n",pszName);
	RFile cf;

	FILE*	fp = fopen(pszName,"rb");

	if (!fp) 
		{
		TRACE("FNT FILE NOT FOUND\n");
		return FAILURE;
		}

	fscanf(fp,"%s",pszCompare);
	if ( (strcmp(pszCompare,"FONTFILE")!=0) && (strcmp(pszCompare,"FONTFIL2")!=0) )
		{
		TRACE("ERROR: Font type not supported\n");
		return FAILURE;
		}

	// get name
	fscanf(fp,"%s",pszCompare);
	m_pszFontName = (char*)calloc(1,strlen(pszCompare)+1);// only true overwrite
	strcpy(m_pszFontName,pszCompare);
	// get max cell height:
	fscanf(fp,"%hd\n",&s1); //m_sMaxCellHeight should be logical
	RImage*	pimLetter = NULL;
	long	lCodeLen;
	UCHAR	ucCheckSum;
	long	ucCheck;
	UCHAR*	pCode;
	RSpecialFSPR1* pInfo = NULL;

	// Note: 'K' will denote a kerning table which should
	// be applied to the font just loaded.
	//
	while ((!sDone) && fscanf(fp,"%c",&c))
		switch(c)
			{
			case 'F':
				fscanf(fp,"%hd\n",&s1);
				TRACE("CACHED FONT SIZE %hd\n",s1);
				fscanf(fp,"%hd\n",&s1);
				TRACE("CACHED FONT WIDTH %hd\n",s1);

			break;

			case 'L':
				pimLetter = new RImage;
				pimLetter->CreateImage(0,0,RImage::FSPR1);
				pInfo = new RSpecialFSPR1;
				pimLetter->m_pSpecialMem = pimLetter->m_pSpecial = (UCHAR*) pInfo;

				fscanf(fp,"%hd\n",&pInfo->m_u16ASCII);
				//TRACE("Adding letter:{%c}\n",(char)pInfo->usASCII);
				
				fscanf(fp,"%ld\n",&pimLetter->m_sWidth);
				fscanf(fp,"%ld\n",&pimLetter->m_sHeight);

				/* not currently supported:
				fscanf(fp,"%hd\n",&pimLetter->sBaseH);
				fscanf(fp,"%hd\n",&pimLetter->sOffsetL);
				fscanf(fp,"%hd\n",&pimLetter->sOffsetT);
				fscanf(fp,"%hd\n",&pimLetter->sOffsetW);
				fscanf(fp,"%hd\n",&pimLetter->sOffsetH);
				*/
				// but leave space:
				short sDummy;
				fscanf(fp,"%hd\n",&sDummy);
				fscanf(fp,"%hd\n",&sDummy);
				fscanf(fp,"%hd\n",&sDummy);
				fscanf(fp,"%hd\n",&sDummy);
				fscanf(fp,"%hd\n",&sDummy);

				ucCheck = (UCHAR)0;
				fscanf(fp,"%02x\n",&l1);
				ucCheckSum = (UCHAR) l1;
				fscanf(fp,"%ld\n",&lCodeLen);

				pInfo->m_lSize = lCodeLen;
				pCode = pInfo->m_pCode = (UCHAR*)calloc(1,lCodeLen+2);// for debugging

				for (i=0;i<lCodeLen;i++,pCode++)
					{
					//fscanf(fp,"%c",pCode);
					*pCode = (UCHAR)fgetc(fp);
					ucCheck ^= (*pCode);
					}
				// AN extension to FSPR1 uses FFFF as end of line:
				*pCode++ = UCHAR(0xff);
				*pCode++ = UCHAR(0xff);

				if (ucCheck != ucCheckSum)
					{
					TRACE("ERROR: character %02x, parity error\n"
							"%02x vs %02x\n",
							(unsigned long)pInfo->m_u16ASCII,
							(unsigned long)ucCheckSum,(unsigned short)ucCheck);
					fscanf(fp,"%c",&c);	// remove trailing newline...
					TRACE("Next character is %02x\n",(unsigned long) c);
					sDone = TRUE;
					break;
					}
				
				AddLetter(pimLetter);
				// remove trailer:
				fscanf(fp,"%c",&c);	// remove trailing newline...
			break;

			default:
			sDone = TRUE;
			}

	fclose(fp);
	return SUCCESS;
	}

// Change this into a circular buffer!  Then it's fast and has mximum memory possible!
RPrint::RPrint()
	{
	m_pBuf = m_buffer;
	m_clrBKD = (UCHAR)0;
	m_clrFGD = (UCHAR)255;
	m_fnCurrent = NULL;
	m_pFontSize = NULL;
	m_pCurFracX = m_pCurFracY = NULL;
	m_sCurX = 0;
	m_sCurY = 0;
	//rspNameBuffers(&m_pimTarget); // Get the main buffer!  (Can't Do Yet cause Blue isn't initialized!)
	m_pimTarget = NULL;
	m_sCellH = 0;
	m_sFORMAT = 0;
	m_sL = m_sT = 0;
	m_sW = 0; //(short)m_pimTarget->lWidth; // can't know default yet!
	m_sH = 0; //(short)m_pimTarget->lHeight;
	m_sTabW = m_sBold = m_sItalic = m_sSpace = 0;
	m_fTabW = m_fItalic = m_fBold = (float)0;
	m_fWide = (float)1.0;
	for (short i=0;i<cgsMAX_FONT_SIZE;i++) m_psItalic[i] = 0;
	}

RPrint::~RPrint()
	{
	//if (m_pCurFracX != NULL) free (m_pCurFracX);
	//if (m_pCurFracY != NULL) free (m_pCurFracY);
	}

RPrint&	RPrint::debug()
	{
	*m_pBuf = '\0';
	TRACE("BUFFER IS:\n%s<==\n",m_buffer);
	m_pBuf = m_buffer;

	return *this;
	}

//===========================================
/*
void	RPrint::SetNarrow(float fW) // don't use this way!
	{
	short	sHeight;

	if (fW == 1.0)
		{
		m_pCurFracX = NULL;
		return;
		}

	sHeight = (short) (fW * (float)m_sCellH); // use as a reference!
	m_pCurFracX = u16fStrafe256(m_sCellH,sHeight);
	}
*/

void	RPrint::Clear()
	{
	m_pCurBuf = m_pBuf = m_buffer;
	}

void	RPrint::SetColumn(short sX,short sY,short sW,short sH,RImage* pimDst)
	{
	m_sL = sX;
	m_sT = sY;

	if (sW) m_sW = sW;
	if (sH) m_sH = sH;
	if (pimDst) 
		{
		if ((m_pimTarget == NULL) && (m_sW == 0) && (m_sH == 0))// set w and h:
			{
			m_sW = pimDst->m_sWidth;
			m_sH = pimDst->m_sHeight;
			}

		m_pimTarget = pimDst;
		}
	}

void	RPrint::SetFont(RFontOld* pFnt,short	sHeight)
	{
	// choose a font to draw with!
	if (pFnt == NULL)
		{
		TRACE("set: error, null font!\n");
		return;
		}

	if (sHeight > pFnt->m_sMaxCellHeight)
		{
		TRACE("set: cannot yet magnify fonts!\n"
			"Max height of font is %hd\n",
			pFnt->m_sMaxCellHeight);
		return;
		}
	
	m_pFontSize = NULL;
	m_fnCurrent = pFnt;
	m_sCellH = sHeight;
	FontNode*	pSize = pFnt->m_pFontList;
	FontNode*	pMax = NULL;

	while (pSize)
		{
		if (pSize->sCellHeight == sHeight) 
			{
			m_pFontSize = pSize;
			break;
			}

		if (pSize->sCellHeight == pFnt->m_sMaxCellHeight)
			pMax = pSize;

		pSize = pSize->pNext;
		}

	if (m_pFontSize) // don't need to scale!
		m_pCurFracY = NULL;
	else					// DO need to scale!
		{
		m_pFontSize = pMax;
		//m_pCurFracY = u16fStrafe256(m_pFontSize->sCellHeight,sHeight);
		//m_pCurFracY = u16fStrafe256(sHeight,m_pFontSize->sCellHeight);
		}
	
	// deal with horizontal scaling:
	/*
	if (fNarrow == 1.0) m_pCurFracX = NULL; // use same for both!
	else
		{
		SetNarrow(fNarrow); // do the calculation stuff!
		}
	*/
	// Now you must update parameters that are already in place or clear them:
	m_sFORMAT = 0;
	if (sHeight > cgsMAX_FONT_SIZE)
		{
		TRACE("SetFont: Fatal error... Height bigger than cgsMAX_FONT_SIZE!\n");
		}
	}

//===========================================
//=========  Let's Print!!!   ===============
//===========================================

short RPrint::LineFeed()
	{
	m_sCurX = m_sL;
	m_sCurY += m_sCellH;

	if ((m_sCurY + m_sCellH) > (m_sT + m_sH)) 
		{
		// save remaining buffer:
		strcpy(m_buffer,m_pCurBuf - 1);
		m_pBuf = m_buffer + strlen(m_buffer);
		m_pCurBuf = m_buffer;

		return -1;
		}

	return 0;
	}

void	RPrint::print(short sX,short sY,RImage* pimTarget)
	{
	m_sCurX = sX;
	m_sCurY = sY;
	if (pimTarget != NULL) 
		SetColumn(0,0,pimTarget->m_sWidth,
			pimTarget->m_sHeight,pimTarget);
	if (m_sCurX < m_sL) m_sCurX = m_sL;
	if (m_sCurY < m_sL) m_sCurY = m_sL;

	print();
	}

void	RPrint::printC(short sX,short sY,short sW,RImage* pimTarget)
	{
	short sCenterOffset = 0;
	if (pimTarget == NULL) return;

	sCenterOffset = (sW - GetWidth())>>1;
	SetColumn(sX + sCenterOffset,sY);

	print(sX+sCenterOffset,sY,pimTarget);
	}

//============================================================================

void RPrint::SetColor(short sLetter,short sBkd)
	{
	m_clrBKD = (UCHAR) sBkd;
	m_clrFGD = (UCHAR) sLetter;
	}

void	RPrint::SetTab(short sPixNum)
	{
	m_sTabW = sPixNum;
	m_fTabW = (float)sPixNum / (float)m_sCellH;
	}

void	RPrint::SetTab(float fTab) // in height units
	{
	m_fTabW = fTab;
	m_sTabW = (short)(fTab * (float)m_sCellH);
	}

void	RPrint::SetStretch(float fWide) // 1.0 = normal
	{
	m_sATTRIB |= TXT_WIDE;
	m_fWide = fWide;
	}

void	RPrint::SetSpace(float fSpace)
	{
	m_fSpace = fSpace;
	m_sSpace = (short)(fSpace * m_sCellH);
	}

void	RPrint::SetSpace(short sSpace) // in pixels
	{
	m_sSpace = sSpace;
	m_fSpace = (float)sSpace / (float)m_sCellH;
	}

void	RPrint::SetItalic(float fItalic)
	{
	m_fItalic = fItalic;
	m_sItalic = (short)(fItalic * m_sCellH);
	SetItalic(m_sItalic); // do the logic down there!
	}

void	RPrint::SetItalic(short sItalic) // in pixels
	{
	short i;
	/*
	static sLastItalic = -9999;

	if (sItalic == sLastItalic) return; // abort if no itlic change!
	sLastItalic = sItalic;
	*/

	if (sItalic == 0)
		{
		m_sItalic = 0;
		m_fItalic = (float)0.0;
		for (i=0;i<cgsMAX_FONT_SIZE;i++) m_psItalic[i] = 0;

		return;
		}

	// NOTE:  The italics are based on the SOURCE image size, so
	// you DON'T need to redo this for a simple scaling, only a source
	// font change!

	m_sItalic = sItalic;
	m_fItalic = (float)sItalic / (float)m_sCellH;
	m_sATTRIB |= TXT_ITALIC;

	// populate the offset array based on current font height!
	u16Frac	fr16CurOff = {0};
	short sAbsOff = ABS(sItalic);
	u16Frac	fr16Inc = {0};
	MakeProper(fr16Inc,sAbsOff,m_sCellH);
	fr16CurOff.frac = (m_sCellH>>1); // for a clean look go by center points.
	if (sItalic > 0)		
		for (i=0;i<m_sCellH;i++)
			{
			Add(fr16CurOff,fr16Inc,m_sCellH);
			m_psItalic[i] = sAbsOff - fr16CurOff.delta;
			}
	else
		for (i=0;i<m_sCellH;i++)
			{
			Add(fr16CurOff,fr16Inc,m_sCellH);
			m_psItalic[i] = fr16CurOff.delta;
			}
	}

// Does NOT include extra kerning width!
void	RPrint::GetCell(RImage* pimLetter,short* psH,
							 short* psCellW, // all effects INCLUDING addspace
							 short* psStretchW // ONLY stretch effect included
							 )
	{
	if (pimLetter == NULL)
		{
		*psH = m_sCellH;
		if (psCellW) *psCellW = 0;
		if (psStretchW) *psStretchW = 0;

		return;
		}

	short w,h;
	h = pimLetter->m_sHeight;
	w = pimLetter->m_sWidth;

	// scale proportionally from pim description to current height:
	w = (short) ((long)m_sCellH * (long)w / h);
	h = m_sCellH;
	if (psH) *psH = h;

	if (m_sATTRIB & TXT_WIDE) w = (short) (m_fWide * w);
	if (psStretchW) *psStretchW = w;

	if (m_sATTRIB & TXT_BOLD) w += m_sBold;
	if (m_sATTRIB & TXT_ITALIC) w += ABS(m_sItalic); // either way adds the same.
	w += m_sSpace;
	if (psCellW) *psCellW = w;
	}

//============================================================================


// handle special characters, etc....
// deals with printable characters only:
short	RPrint::GetChar()
	{
	if (m_pCurBuf == m_pBuf) return 0;
	short sChar;

	do	sChar = *(m_pCurBuf++);
	while ( (m_pFontSize->pLetters[sChar] == NULL) && // remove non-printables
		(m_pCurBuf <= m_pBuf) );

	if (m_pCurBuf > m_pBuf) return 0;
	return sChar;
	}

// SIMPLE (no multiLine) logic:
// Goes from current character on...
// Goes from current position right
//
void	RPrint::PrintLine(short sNumChar,short /*sJustDelta*/,
		short /*sJustFraction*/,short /*sDen*/)
	{
	short sX = m_sCurX,sFracX = 0;
	short	sChar = 0;
	RImage* pimCur;
	short sCellW,sStretchW,sH;
	RRect	rCol;

	rCol.sX = m_sL;
	rCol.sY = m_sT;
	rCol.sW = m_sW;
	rCol.sH = m_sH;
	
	for (short i=0;i<sNumChar;i++)
		{
		sChar = GetChar();
		pimCur = m_pFontSize->pLetters[sChar];
		if (pimCur == NULL) continue; // Font has no letter

		GetCell(pimCur,&sH,&sCellW,&sStretchW);

		m_sCurX = sX;
		/******************* Must modernize!  //+
		sW = pimCur->Draw(-1,m_sCellH,m_pimTarget,m_sCurX,m_sCurY,
			m_clrBKD,m_clrFGD,-1,m_pCurFracX,m_pCurFracY);
		*/
	//	_rspBlit((ULONG)m_clrFGD,(ULONG)m_clrBKD,pimCur,m_pimTarget,
	//		m_sCurX,m_sCurY,sW,sH,0,m_pCurFracY);//,m_pCurFracY,m_pCurFracX);
		
		/* old...
		rspBlit((ULONG)m_clrFGD,(ULONG)m_clrBKD,pimCur,m_pimTarget,
			m_sCurX,m_sCurY,sStretchW,sH,&rCol,0,m_psItalic);
		*/
		// New...can't clip...
		rspBlit((ULONG)m_clrFGD,pimCur,m_pimTarget,
			m_sCurX,m_sCurY,sStretchW,sH,m_psItalic);

		sX += sCellW;
		/*
		sX += sJustDelta;
		sFracX += sJustFraction;
		if (sFracX >= sDen)
			{
			sX ++;
			sFracX -= sDen;
			}
			*/
		}
	}


RPrint& RPrint::print()
	{
	*m_pBuf = '\0';
	short	sChar = 0;
	short	sCellW = 0,sStretchW = 0;

	if (m_pimTarget == NULL) 
		{
		rspNameBuffers(&m_pimTarget); // Get the main buffer!
		m_sW = m_pimTarget->m_sWidth; // can't know default yet!
		m_sH = m_pimTarget->m_sHeight;
		}

	if (m_pCurBuf == m_pBuf) {Clear(); return *this;} // nothing to print!

	if (m_fnCurrent == NULL)
		{
		TRACE("print: no font set!\n");
		return *this;
		}
	
	// off bottom of buffer!
	if ((m_sCurY + m_sCellH) > (m_sT + m_sH))
		{
		m_pCurBuf = m_buffer;
		return *this;
		}

	m_pCurBuf = m_buffer;
	RRect	rCol;

	rCol.sX = m_sL;
	rCol.sY = m_sT;
	rCol.sW = m_sW;
	rCol.sH = m_sH;

	while (*m_pCurBuf)
		{
		// print each char;
		sChar = (short)((UCHAR) *(m_pCurBuf++)); // Must force it positive!

		// look for special characters:
		switch ( (UCHAR) sChar)
			{
			case '\n': // do newline + cr
				if (LineFeed()) return *this;

				continue;

			case '\r': // just a return
				m_sCurX = m_sL;
				continue;

			case '\b':
				TRACE("print: Backspace NYI\n");
				continue;

			case '\t':
				m_sCurX += m_sTabW;
				continue;
			}

		// Draw the letter!
		// Scan ahead for line wrap:
		short sH;


		RImage* pimCur = m_pFontSize->pLetters[sChar];
		GetCell(pimCur,&sH,&sCellW,&sStretchW);

		// Scan ahead for line wrap:
		if ( (m_sCurX + sCellW) > (m_sL + m_sW) )
			{
			if (	LineFeed()) return *this;
			}

		if (pimCur == NULL) 
			continue; // Font has no letter
		
		/* We cannot currently support bold!
		sCellW = pimCur->Draw(-1,m_sCellH,m_pimTarget,m_sCurX,m_sCurY,
			m_clrBKD,m_clrFGD,-1,m_pCurFracX,m_pCurFracY);
			*/
		//_rspBlit((ULONG)m_clrFGD,(ULONG)m_clrBKD,pimCur,m_pimTarget,
		//	m_sCurX,m_sCurY,sW,sH,0,m_pCurFracY);//,m_pCurFracY,m_pCurFracX);
		// Pull out interspacing:

		/* Old style...
		rspBlit((ULONG)m_clrFGD,(ULONG)m_clrBKD,pimCur,m_pimTarget,
			m_sCurX,m_sCurY,sStretchW,sH,&rCol,m_sSpace,m_psItalic);//,m_pCurFracY,m_pCurFracX);
		*/
		// New style (can't clip...
		rspBlit((ULONG)m_clrFGD,pimCur,m_pimTarget,
			m_sCurX,m_sCurY,sStretchW,sH,m_psItalic);

		m_sCurX += sCellW - ABS(m_sItalic);
		}

	Clear(); // clear the text buffer!
	return *this;
	}

RPrint&	RPrint::operator|(char* psz)
	{
	char*	pIn = psz;

	while (*pIn)
		*(m_pBuf++) = *(pIn++);

	*m_pBuf = 0;
	return *this;
	}

RPrint&	RPrint::operator<<(char* psz)
	{
	char*	pIn = psz;

	while (*pIn)
		*(m_pBuf++) = *(pIn++);

	*m_pBuf = 0;
	return *this;
	}


RPrint& RPrint::operator|(short	in)
	{
	sprintf(m_temp,"%hd",in);
	return (*this) | m_temp;
	};


RPrint& RPrint::operator|(long	in)
	{
	sprintf(m_temp,"%ld",in);
	return (*this) | m_temp;
	};

RPrint& RPrint::operator|(float	in)
	{
	sprintf(m_temp,"%hg",in);
	return (*this) | m_temp;
	};

RPrint& RPrint::operator|(double	in)
	{
	sprintf(m_temp,"%lg",in);
	return (*this) | m_temp;
	};

RPrint& RPrint::operator|(char	in)
	{
	sprintf(m_temp,"%c",in);
	return (*this) | m_temp;
	};

// Makes it easy for the app to ge the current characteristics:
// This INCLUDES extra space padding?
//
void	RPrint::GetCell(short sASCII,short &w,short &h)
	{
	RImage* pimLetter = m_pFontSize->pLetters[sASCII];
	GetCell(pimLetter,&h,&w);
	}

// current buffer, current settings...
//
short	RPrint::GetWidth()
	{
	char* pcPos = m_buffer;
	short sW = 0;
	short sTotW = 0;

	while (*pcPos)
		{
		GetCell(m_pFontSize->pLetters[(UCHAR)(*(pcPos++))],NULL,&sW);
		sTotW += sW;
		}

	return sTotW;
	}


// dumb declarators!
/* let's try it as not being static:

char	RPrint::m_buffer[4096];
char*	RPrint::m_pBuf; // open
char*	RPrint::m_pCurBuf; // open
char	RPrint::m_temp[256];
*/
